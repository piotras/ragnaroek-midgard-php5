#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
# define php_mgd_function_state zend_function_state
# define php_mgd_function_state_ptr() EG(current_execute_data)->function_state
#else
# define Z_SET_ISREF_P(ptr) (ptr)->is_ref = 1
# define php_mgd_function_state zend_function_state*
# define php_mgd_function_state_ptr() EG(function_state_ptr)
# define zend_parse_parameters_none() zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")
#endif
