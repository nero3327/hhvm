/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_INTL_H_
#define incl_HPHP_EXT_INTL_H_

// >>>>>> Generated by idl.php. Do NOT modify. <<<<<<

#include <runtime/base/base_includes.h>
#include <runtime/base/zend/zend_collator.h>
#include <unicode/ucol.h> // icu
namespace HPHP {
extern const int64_t q_Collator$$SORT_REGULAR;
}
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int64_t f_intl_get_error_code();
String f_intl_get_error_message();
String f_intl_error_name(int64_t error_code);
bool f_intl_is_failure(int64_t error_code);
Variant f_collator_asort(CVarRef obj, VRefParam arr, int64_t sort_flag = q_Collator$$SORT_REGULAR);
Variant f_collator_compare(CVarRef obj, CStrRef str1, CStrRef str2);
Variant f_collator_create(CStrRef locale);
Variant f_collator_get_attribute(CVarRef obj, int64_t attr);
Variant f_collator_get_error_code(CVarRef obj);
Variant f_collator_get_error_message(CVarRef obj);
Variant f_collator_get_locale(CVarRef obj, int64_t type = 0);
Variant f_collator_get_strength(CVarRef obj);
Variant f_collator_set_attribute(CVarRef obj, int64_t attr, int64_t val);
Variant f_collator_set_strength(CVarRef obj, int64_t strength);
Variant f_collator_sort_with_sort_keys(CVarRef obj, VRefParam arr);
Variant f_collator_sort(CVarRef obj, VRefParam arr, int64_t sort_flag = q_Collator$$SORT_REGULAR);
Variant f_idn_to_ascii(CStrRef domain, int64_t options = 0, int64_t variant = 0, VRefParam idna_info = uninit_null());
Variant f_idn_to_unicode(CStrRef domain, int64_t options = 0, int64_t variant = 0, VRefParam idna_info = uninit_null());
Variant f_idn_to_utf8(CStrRef domain, int64_t options = 0, int64_t variant = 0, VRefParam idna_info = uninit_null());
extern const int64_t q_Collator$$SORT_REGULAR;
extern const int64_t q_Collator$$SORT_NUMERIC;
extern const int64_t q_Collator$$SORT_STRING;
extern const int64_t q_Collator$$FRENCH_COLLATION;
extern const int64_t q_Collator$$ALTERNATE_HANDLING;
extern const int64_t q_Collator$$CASE_FIRST;
extern const int64_t q_Collator$$CASE_LEVEL;
extern const int64_t q_Collator$$NORMALIZATION_MODE;
extern const int64_t q_Collator$$STRENGTH;
extern const int64_t q_Collator$$HIRAGANA_QUATERNARY_MODE;
extern const int64_t q_Collator$$NUMERIC_COLLATION;
extern const int64_t q_Collator$$DEFAULT_VALUE;
extern const int64_t q_Collator$$PRIMARY;
extern const int64_t q_Collator$$SECONDARY;
extern const int64_t q_Collator$$TERTIARY;
extern const int64_t q_Collator$$DEFAULT_STRENGTH;
extern const int64_t q_Collator$$QUATERNARY;
extern const int64_t q_Collator$$IDENTICAL;
extern const int64_t q_Collator$$OFF;
extern const int64_t q_Collator$$ON;
extern const int64_t q_Collator$$SHIFTED;
extern const int64_t q_Collator$$NON_IGNORABLE;
extern const int64_t q_Collator$$LOWER_FIRST;
extern const int64_t q_Collator$$UPPER_FIRST;

///////////////////////////////////////////////////////////////////////////////
// class Collator

FORWARD_DECLARE_CLASS_BUILTIN(Collator);
class c_Collator : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(Collator, Collator, ObjectData)

  // need to implement
  public: c_Collator(VM::Class* cls = c_Collator::s_cls);
  public: ~c_Collator();
  public: void t___construct(CStrRef locale);
  public: bool t_asort(VRefParam arr, int64_t sort_flag = q_Collator$$SORT_REGULAR);
  public: Variant t_compare(CStrRef str1, CStrRef str2);
  public: static Variant ti_create(CStrRef locale);
  public: int64_t t_getattribute(int64_t attr);
  public: int64_t t_geterrorcode();
  public: String t_geterrormessage();
  public: String t_getlocale(int64_t type = 0);
  public: int64_t t_getstrength();
  public: bool t_setattribute(int64_t attr, int64_t val);
  public: bool t_setstrength(int64_t strength);
  public: bool t_sortwithsortkeys(VRefParam arr);
  public: bool t_sort(VRefParam arr, int64_t sort_flag = q_Collator$$SORT_REGULAR);

 private:
  String     m_locale;
  UCollator *m_ucoll;
  intl_error m_errcode;
};
extern const int64_t q_Locale$$ACTUAL_LOCALE;
extern const int64_t q_Locale$$VALID_LOCALE;

///////////////////////////////////////////////////////////////////////////////
// class Locale

FORWARD_DECLARE_CLASS_BUILTIN(Locale);
class c_Locale : public ExtObjectData {
 public:
  DECLARE_CLASS(Locale, Locale, ObjectData)

  // need to implement
  public: c_Locale(VM::Class* cls = c_Locale::s_cls);
  public: ~c_Locale();
  public: void t___construct();
};
extern const int64_t q_Normalizer$$NONE;
extern const int64_t q_Normalizer$$FORM_D;
extern const int64_t q_Normalizer$$NFD;
extern const int64_t q_Normalizer$$FORM_KD;
extern const int64_t q_Normalizer$$NFKD;
extern const int64_t q_Normalizer$$FORM_C;
extern const int64_t q_Normalizer$$NFC;
extern const int64_t q_Normalizer$$FORM_KC;
extern const int64_t q_Normalizer$$NFKC;

///////////////////////////////////////////////////////////////////////////////
// class Normalizer

FORWARD_DECLARE_CLASS_BUILTIN(Normalizer);
class c_Normalizer : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(Normalizer, Normalizer, ObjectData)

  // need to implement
  public: c_Normalizer(VM::Class* cls = c_Normalizer::s_cls);
  public: ~c_Normalizer();
  public: void t___construct();
  public: static Variant ti_isnormalized(CStrRef input, int64_t form = q_Normalizer$$FORM_C);
  public: static Variant ti_normalize(CStrRef input, int64_t form = q_Normalizer$$FORM_C);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_INTL_H_
