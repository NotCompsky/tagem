function $$$regexp_esc_for_mysql(s){
	return s.replace(/[.*+?^${}()|[\]\\]/g, '\\\\$&');
}
