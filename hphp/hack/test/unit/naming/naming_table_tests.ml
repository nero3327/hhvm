(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

(* These are basically the same tests as in
 * test/integration_ml/saved_state/test_naming_table_sqlite_fallback.ml, but
 * as stripped down to just the basics as possible to make finding the root
 * cause of test failures easier. *)

open Core_kernel

module Types_pos_asserter = Asserter.Make_asserter (struct
  type t = FileInfo.pos * Naming_types.kind_of_type

  let to_string (pos, type_of_type) =
    Printf.sprintf
      "(%s, %d)"
      (FileInfo.show_pos pos)
      (Naming_types.kind_of_type_to_enum type_of_type)

  let is_equal = ( = )
end)

module Pos_asserter = Asserter.Make_asserter (struct
  type t = FileInfo.pos

  let to_string pos = Printf.sprintf "(%s)" (FileInfo.show_pos pos)

  let is_equal = ( = )
end)

let files =
  [
    ("foo.php", {|<?hh
    class Foo {}
  |});
    ("bar.php", {|<?hh
    function bar(): void {}
  |});
    ("baz.php", {|<?hh
    type Baz = Foo;
  |});
    ("qux.php", {|<?hh
    const int Qux = 5;
  |});
  ]

let write_and_parse_test_files () =
  let files =
    List.map files ~f:(fun (fn, contents) ->
        (Relative_path.from_root fn, contents))
  in
  List.iter files ~f:(fun (fn, contents) ->
      let fn = Path.make (Relative_path.to_absolute fn) in
      let dir = Path.dirname fn in
      Disk.mkdir_p (Path.to_string dir);
      Disk.write_file ~file:(Path.to_string fn) ~contents);
  let (file_infos, errors, failed_parsing) =
    Parsing_service.go
      None
      Relative_path.Set.empty
      ~get_next:(MultiWorker.next None (List.map files ~f:fst))
      ParserOptions.default
      ~trace:true
  in
  if not (Errors.is_empty errors) then (
    Errors.iter_error_list
      (fun e ->
        List.iter (Errors.to_list e) ~f:(fun (pos, msg) ->
            eprintf "%s: %s\n" (Pos.string (Pos.to_absolute pos)) msg))
      errors;
    failwith "Expected no errors from parsing."
  );
  if failed_parsing <> Relative_path.Set.empty then
    failwith "Expected all files to pass parsing.";
  Naming_table.create file_infos

let run_naming_table_test f =
  Tempfile.with_real_tempdir (fun path ->
      Relative_path.set_path_prefix
        Relative_path.Root
        (Path.concat path "root/");
      let config =
        SharedMem.
          {
            global_size = 1024;
            heap_size = 1024 * 1024;
            dep_table_pow = 16;
            hash_table_pow = 10;
            shm_dirs = [];
            shm_min_avail = 0;
            log_level = 0;
            sample_rate = 0.0;
          }
      in
      let (_ : SharedMem.handle) = SharedMem.init config ~num_workers:0 in
      let unbacked_naming_table = write_and_parse_test_files () in
      let db_name = Path.to_string (Path.concat path "naming_table.sqlite") in
      let save_results = Naming_table.save unbacked_naming_table db_name in
      Asserter.Int_asserter.assert_equals
        8
        Naming_sqlite.(save_results.files_added + save_results.symbols_added)
        "Expected to add eight rows (four files and four symbols)";
      let backed_naming_table = Naming_table.load_from_sqlite db_name in
      let ctx =
        Provider_context.empty_for_test ~tcopt:TypecheckerOptions.default
      in
      f ~ctx ~unbacked_naming_table ~backed_naming_table ~db_name;
      true)

let test_get_pos () =
  run_naming_table_test
    (fun ~ctx:_ ~unbacked_naming_table:_ ~backed_naming_table:_ ~db_name:_ ->
      Types_pos_asserter.assert_option_equals
        (Some
           ( FileInfo.File (FileInfo.Class, Relative_path.from_root "foo.php"),
             Naming_types.TClass ))
        (Naming_heap.Types.get_pos "\\Foo")
        "Check for class type";
      Pos_asserter.assert_option_equals
        (Some (FileInfo.File (FileInfo.Fun, Relative_path.from_root "bar.php")))
        (Naming_heap.Funs.get_pos "\\bar")
        "Check for function";
      Types_pos_asserter.assert_option_equals
        (Some
           ( FileInfo.File (FileInfo.Typedef, Relative_path.from_root "baz.php"),
             Naming_types.TTypedef ))
        (Naming_heap.Types.get_pos "\\Baz")
        "Check for typedef type";
      Pos_asserter.assert_option_equals
        (Some
           (FileInfo.File (FileInfo.Const, Relative_path.from_root "qux.php")))
        (Naming_heap.Consts.get_pos "\\Qux")
        "Check for const")

let test_get_canon_name () =
  run_naming_table_test
    (fun ~ctx ~unbacked_naming_table:_ ~backed_naming_table:_ ~db_name:_ ->
      (* Since we're parsing but not naming, the canon heap must fall back to the
       files on disk, which is the situation we'd be in when loading from a
       saved state. *)
      Asserter.String_asserter.assert_option_equals
        (Some "\\Foo")
        (Naming_heap.Types.get_canon_name ctx "\\foo")
        "Check for class canon name";
      Asserter.String_asserter.assert_option_equals
        (Some "\\bar")
        (Naming_heap.Funs.get_canon_name ctx "\\bar")
        "Check for function canon name";
      Asserter.String_asserter.assert_option_equals
        (Some "\\Baz")
        (Naming_heap.Types.get_canon_name ctx "\\baz")
        "Check for typedef canon name")

let test_remove () =
  run_naming_table_test
    (fun ~ctx:_ ~unbacked_naming_table ~backed_naming_table ~db_name:_ ->
      let foo_path = Relative_path.from_root "foo.php" in
      assert (
        Naming_table.get_file_info unbacked_naming_table foo_path
        |> Option.is_some );
      let unbacked_naming_table =
        Naming_table.remove unbacked_naming_table foo_path
      in
      assert (
        Naming_table.get_file_info unbacked_naming_table foo_path
        |> Option.is_none );

      assert (
        Naming_table.get_file_info backed_naming_table foo_path
        |> Option.is_some );
      let backed_naming_table =
        Naming_table.remove backed_naming_table foo_path
      in
      assert (
        Naming_table.get_file_info backed_naming_table foo_path
        |> Option.is_none ))

let test_get_sqlite_paths () =
  run_naming_table_test
    (fun ~ctx:_ ~unbacked_naming_table:_ ~backed_naming_table ~db_name ->
      Asserter.String_asserter.assert_option_equals
        (Some db_name)
        (Naming_table.get_reverse_naming_fallback_path ())
        "get_reverse_naming_fallback_path should return the expected value";

      Asserter.String_asserter.assert_option_equals
        (Some db_name)
        (Naming_table.get_forward_naming_fallback_path backed_naming_table)
        "get_forward_naming_fallback_path should return the expected value")

let test_local_changes () =
  run_naming_table_test
    (fun ~ctx:_ ~unbacked_naming_table:_ ~backed_naming_table ~db_name ->
      let a_name = "CONST_IN_A" in

      let a_file = Relative_path.from_root "a.php" in
      let a_pos = FileInfo.File (FileInfo.Const, a_file) in
      let a_file_info =
        FileInfo.{ FileInfo.empty_t with consts = [(a_pos, a_name)] }
      in
      let backed_naming_table =
        Naming_table.update backed_naming_table a_file a_file_info
      in
      let changes_since_baseline_path = "/tmp/base_plus_changes" in
      Naming_table.save_changes_since_baseline
        backed_naming_table
        changes_since_baseline_path;
      let changes_since_baseline =
        Marshal.from_string (Disk.cat changes_since_baseline_path) 0
      in
      Asserter.Relative_path_asserter.assert_list_equals
        [a_file]
        (Naming_table.get_files_changed_since_baseline changes_since_baseline)
        "Expected files changed since baseline to be correct";
      let backed_naming_table' =
        Naming_table.load_from_sqlite_with_changes_since_baseline
          changes_since_baseline
          db_name
      in
      let a_file_info' =
        Option.value_exn
          (Naming_table.get_file_info backed_naming_table' a_file)
      in
      Asserter.Bool_asserter.assert_equals
        true
        (a_file_info = a_file_info')
        "Expected file info to be found in the naming table";

      let a_pos' = Option.value_exn (Naming_heap.Consts.get_pos a_name) in
      Asserter.Bool_asserter.assert_equals
        true
        (a_pos = a_pos')
        "Expected position of constant to be found in the naming table")

let () =
  Unit_test.run_all
    [
      ("test_get_pos", test_get_pos);
      ("test_get_canon_name", test_get_canon_name);
      ("test_remove", test_remove);
      ("test_get_sqlite_paths", test_get_sqlite_paths);
      ("test_local_changes", test_local_changes);
    ]
