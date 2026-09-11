#ifndef STRING_MAP_NATIVE_STRING_MAP_H
#define STRING_MAP_NATIVE_STRING_MAP_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class StringMapNative : public RefCounted {
	GDCLASS(StringMapNative, RefCounted)

public:
	enum Mode {
		FULL,
		STRING,
	};

private:
	String string;
	PackedByteArray string_mask;
	PackedByteArray comment_mask;
	Dictionary string_map;
	Dictionary quote_map;
	Dictionary bracket_map;
	bool has_errors = false;
	int64_t mode = FULL;

	void _parse(const String &p_text, bool p_print_err);

protected:
	static void _bind_methods();

public:
	static Ref<StringMapNative> create(const String &p_text, int64_t p_mode = FULL, bool p_print_err = false);

	void set_string(const String &p_text);
	String get_string() const;
	void set_string_mask(const PackedByteArray &p_mask);
	PackedByteArray get_string_mask() const;
	void set_comment_mask(const PackedByteArray &p_mask);
	PackedByteArray get_comment_mask() const;
	void set_string_map(const Dictionary &p_map);
	Dictionary get_string_map() const;
	void set_quote_map(const Dictionary &p_map);
	Dictionary get_quote_map() const;
	void set_bracket_map(const Dictionary &p_map);
	Dictionary get_bracket_map() const;
	void set_has_errors(bool p_has_errors);
	bool get_has_errors() const;
	void set_mode(int64_t p_mode);
	int64_t get_mode() const;

	bool index_in_string_or_comment(int64_t p_index) const;
	int64_t get_comment_index(int64_t p_from = 0) const;
	String get_line_at_index(int64_t p_index) const;
	Array get_strings() const;
	int64_t get_tightest_bracket_set(int64_t p_idx, const String &p_bracket_type = "") const;
};

} // namespace godot

VARIANT_ENUM_CAST(StringMapNative::Mode);

#endif // STRING_MAP_NATIVE_STRING_MAP_H
