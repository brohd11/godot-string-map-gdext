#include "string_map.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <string>
#include <vector>

namespace godot {

void StringMapNative::_bind_methods() {
	ClassDB::bind_static_method("StringMapNative", D_METHOD("create", "text", "mode", "print_err"), &StringMapNative::create, DEFVAL(FULL), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("set_string", "string"), &StringMapNative::set_string);
	ClassDB::bind_method(D_METHOD("get_string"), &StringMapNative::get_string);
	ClassDB::bind_method(D_METHOD("set_string_mask", "string_mask"), &StringMapNative::set_string_mask);
	ClassDB::bind_method(D_METHOD("get_string_mask"), &StringMapNative::get_string_mask);
	ClassDB::bind_method(D_METHOD("set_comment_mask", "comment_mask"), &StringMapNative::set_comment_mask);
	ClassDB::bind_method(D_METHOD("get_comment_mask"), &StringMapNative::get_comment_mask);
	ClassDB::bind_method(D_METHOD("set_string_map", "string_map"), &StringMapNative::set_string_map);
	ClassDB::bind_method(D_METHOD("get_string_map"), &StringMapNative::get_string_map);
	ClassDB::bind_method(D_METHOD("set_quote_map", "quote_map"), &StringMapNative::set_quote_map);
	ClassDB::bind_method(D_METHOD("get_quote_map"), &StringMapNative::get_quote_map);
	ClassDB::bind_method(D_METHOD("set_bracket_map", "bracket_map"), &StringMapNative::set_bracket_map);
	ClassDB::bind_method(D_METHOD("get_bracket_map"), &StringMapNative::get_bracket_map);
	ClassDB::bind_method(D_METHOD("set_has_errors", "has_errors"), &StringMapNative::set_has_errors);
	ClassDB::bind_method(D_METHOD("get_has_errors"), &StringMapNative::get_has_errors);
	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &StringMapNative::set_mode);
	ClassDB::bind_method(D_METHOD("get_mode"), &StringMapNative::get_mode);

	ClassDB::bind_method(D_METHOD("index_in_string_or_comment", "index"), &StringMapNative::index_in_string_or_comment);
	ClassDB::bind_method(D_METHOD("get_comment_index", "from"), &StringMapNative::get_comment_index, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_line_at_index", "index"), &StringMapNative::get_line_at_index);
	ClassDB::bind_method(D_METHOD("get_strings"), &StringMapNative::get_strings);
	ClassDB::bind_method(D_METHOD("get_tightest_bracket_set", "idx", "bracket_type"), &StringMapNative::get_tightest_bracket_set, DEFVAL(""));

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "string"), "set_string", "get_string");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "string_mask"), "set_string_mask", "get_string_mask");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "comment_mask"), "set_comment_mask", "get_comment_mask");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "string_map"), "set_string_map", "get_string_map");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "quote_map"), "set_quote_map", "get_quote_map");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "bracket_map"), "set_bracket_map", "get_bracket_map");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "has_errors"), "set_has_errors", "get_has_errors");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "FULL,STRING"), "set_mode", "get_mode");

	BIND_ENUM_CONSTANT(FULL);
	BIND_ENUM_CONSTANT(STRING);
}

Ref<StringMapNative> StringMapNative::create(const String &p_text, int64_t p_mode, bool p_print_err) {
	Ref<StringMapNative> sm;
	sm.instantiate();
	sm->string = p_text;
	sm->mode = p_mode;
	sm->_parse(p_text, false);
	return sm;
}

void StringMapNative::_parse(const String &p_text, bool p_print_err) {
	const int64_t text_length = p_text.length();
	string_mask.resize(text_length);
	comment_mask.resize(text_length);
	uint8_t *smask = string_mask.ptrw();
	uint8_t *cmask = comment_mask.ptrw();
	const char32_t *data = p_text.ptr();

	std::vector<int64_t> bracket_stack;
	bool in_comment = false;
	bool in_string = false;
	char32_t quote_char = 0;
	int64_t string_start_index = -1;
	std::u32string current_string;

	int64_t i = -1;
	while (i + 1 < text_length) {
		i += 1;
		const char32_t c = data[i];
		if (in_comment) {
			if (c == U'\n') {
				in_comment = false;
			} else {
				cmask[i] = 1;
			}
		} else if (in_string) {
			smask[i] = 1;
			if (c == U'\\') {
				if (i + 1 < text_length) {
					smask[i + 1] = 1;
				}
				i += 1;
			} else if (c == quote_char) {
				in_string = false;
				quote_map[string_start_index] = i;
				quote_map[i] = string_start_index;
				string_map[string_start_index] = String(current_string.c_str());
				current_string.clear();
				continue;
			}
			current_string += c;
		} else {
			if (c == U'"' || c == U'\'') {
				in_string = true;
				quote_char = c;
				string_start_index = i;
				smask[i] = 1;
			} else if (c == U'#') {
				in_comment = true;
				cmask[i] = 1;
			} else if (mode == FULL) {
				if (c == U'(' || c == U'[' || c == U'{') {
					bracket_stack.push_back(i);
				} else if (c == U')' || c == U']' || c == U'}') {
					if (bracket_stack.empty()) {
						has_errors = true;
						break;
					}
					const int64_t open_idx = bracket_stack.back();
					bracket_stack.pop_back();
					const char32_t open_c = data[open_idx];
					if ((c == U')' && open_c == U'(') || (c == U']' && open_c == U'[') || (c == U'}' && open_c == U'{')) {
						bracket_map[open_idx] = i;
						bracket_map[i] = open_idx;
					} else {
						has_errors = true;
						break;
					}
				}
			}
		}
	}

	if (in_string) {
		if (p_print_err) {
			UtilityFunctions::printerr("Unterminated string starting at index ", string_start_index);
		}
		has_errors = true;
	}
	if (!bracket_stack.empty()) {
		if (p_print_err) {
			UtilityFunctions::printerr("Unclosed opening bracket at index ", bracket_stack.front());
		}
		has_errors = true;
	}
}

void StringMapNative::set_string(const String &p_text) {
	string = p_text;
}

String StringMapNative::get_string() const {
	return string;
}

void StringMapNative::set_string_mask(const PackedByteArray &p_mask) {
	string_mask = p_mask;
}

PackedByteArray StringMapNative::get_string_mask() const {
	return string_mask;
}

void StringMapNative::set_comment_mask(const PackedByteArray &p_mask) {
	comment_mask = p_mask;
}

PackedByteArray StringMapNative::get_comment_mask() const {
	return comment_mask;
}

void StringMapNative::set_string_map(const Dictionary &p_map) {
	string_map = p_map;
}

Dictionary StringMapNative::get_string_map() const {
	return string_map;
}

void StringMapNative::set_quote_map(const Dictionary &p_map) {
	quote_map = p_map;
}

Dictionary StringMapNative::get_quote_map() const {
	return quote_map;
}

void StringMapNative::set_bracket_map(const Dictionary &p_map) {
	bracket_map = p_map;
}

Dictionary StringMapNative::get_bracket_map() const {
	return bracket_map;
}

void StringMapNative::set_has_errors(bool p_has_errors) {
	has_errors = p_has_errors;
}

bool StringMapNative::get_has_errors() const {
	return has_errors;
}

void StringMapNative::set_mode(int64_t p_mode) {
	mode = p_mode;
}

int64_t StringMapNative::get_mode() const {
	return mode;
}

bool StringMapNative::index_in_string_or_comment(int64_t p_index) const {
	if (comment_mask[p_index] == 1) {
		return true;
	}
	if (string_mask[p_index] == 1) {
		return true;
	}
	return false;
}

int64_t StringMapNative::get_comment_index(int64_t p_from) const {
	return comment_mask.find(1, p_from);
}

String StringMapNative::get_line_at_index(int64_t p_index) const {
	int64_t limit = string.length();
	if (p_index > -1) {
		limit = p_index;
	}
	const int64_t max_valid_start = string.length() - 1;
	int64_t beginning_new_line_i = -1;
	if (max_valid_start >= 0) {
		const int64_t actual_from = MIN(limit, max_valid_start);
		beginning_new_line_i = string.rfind("\n", actual_from);
	}
	if (beginning_new_line_i == -1) {
		beginning_new_line_i = 0;
	}
	int64_t end_new_line_i = string.find("\n", p_index);
	if (end_new_line_i == -1) {
		end_new_line_i = string.length() - 1;
	}
	return string.substr(beginning_new_line_i, end_new_line_i - beginning_new_line_i);
}

Array StringMapNative::get_strings() const {
	return string_map.values();
}

int64_t StringMapNative::get_tightest_bracket_set(int64_t p_idx, const String &p_bracket_type) const {
	const bool all_brackets = p_bracket_type.is_empty();
	int64_t open_bracket_index = -1;
	int64_t closed_bracket_index = string.length();
	Array bracket_map_keys = bracket_map.keys();
	bracket_map_keys.sort();

	for (int64_t k = 0; k < bracket_map_keys.size(); k++) {
		const int64_t open = bracket_map_keys[k];
		if (open > p_idx) {
			break;
		}
		const char32_t c = string[open];
		if (!all_brackets && c != p_bracket_type[0]) {
			continue;
		}
		const int64_t close = bracket_map[open];
		if (!(open <= p_idx && close >= p_idx)) {
			continue;
		}
		if (close - open < closed_bracket_index - open_bracket_index) {
			open_bracket_index = open;
			closed_bracket_index = close;
		}
	}

	return open_bracket_index;
}

} // namespace godot
