extends SceneTree

const GDStringMap = preload("res://addons/addon_lib/brohd/alib_runtime/utils/string/string_map.gd")

const INPUT_FILES := [
	"res://addons/addon_lib/brohd/alib_runtime/utils/u_string.gd",
	"res://addons/addon_lib/brohd/alib_runtime/utils/string/string_map.gd",
	"res://addons/addon_lib/brohd/alib_runtime/utils/gdscript/parser/parser_class.gd",
	"res://addons/addon_lib/brohd/alib_runtime/utils/gdscript/parser/gdscript_parser.gd",
]

const EDGE_CASES := [
	"",
	"plain text no strings or brackets",
	"a = \"hello\" # comment\nb = 'x\\'y' (c[d{e}])",
	"unclosed = \"oops\nfoo(bar",
	"mismatched = (]",
	"# only a comment",
	"\"nested 'quotes' inside\" and 'vice \"versa'",
]

var iterations := 300
var warmup := 20
var fail_count := 0

var gd_factory := func(text: String, mode: int): return GDStringMap.new(text, mode)
var cpp_factory := func(text: String, mode: int): return StringMapNative.create(text, mode)


func _init():
	var inputs := _load_inputs()
	_verify(inputs)
	_bench(inputs)
	quit(1 if fail_count > 0 else 0)


func _load_inputs() -> Array:
	var inputs: Array = EDGE_CASES.duplicate()
	for path in INPUT_FILES:
		if FileAccess.file_exists(path):
			inputs.append(FileAccess.get_file_as_string(path))
		else:
			push_warning("missing input: " + path)
	return inputs


func _verify(inputs: Array) -> void:
	for mode in [GDStringMap.Mode.FULL, GDStringMap.Mode.STRING]:
		for text in inputs:
			var gd = gd_factory.call(text, mode)
			var cpp = cpp_factory.call(text, mode)
			_check(gd.string == cpp.string, "string")
			_check(gd.string_mask == cpp.string_mask, "string_mask", text)
			_check(gd.comment_mask == cpp.comment_mask, "comment_mask", text)
			_check(_dicts_equal(gd.string_map, cpp.string_map), "string_map", text)
			_check(_dicts_equal(gd.quote_map, cpp.quote_map), "quote_map", text)
			_check(_dicts_equal(gd.bracket_map, cpp.bracket_map), "bracket_map", text)
			_check(gd.has_errors == cpp.has_errors, "has_errors", text)
			_check(gd.get_strings() == cpp.get_strings(), "get_strings", text)
			for idx in [0, text.length() / 2, maxi(text.length() - 1, 0)]:
				if text.is_empty():
					continue
				_check(gd.index_in_string_or_comment(idx) == cpp.index_in_string_or_comment(idx), "index_in_string_or_comment", text)
				_check(gd.get_line_at_index(idx) == cpp.get_line_at_index(idx), "get_line_at_index", text)
				_check(gd.get_tightest_bracket_set(idx) == cpp.get_tightest_bracket_set(idx), "get_tightest_bracket_set", text)
			_check(gd.get_comment_index() == cpp.get_comment_index(), "get_comment_index", text)
	if fail_count == 0:
		print("VERIFY: all outputs match")


func _dicts_equal(a: Dictionary, b: Dictionary) -> bool:
	if a.size() != b.size():
		return false
	for k in a:
		if not b.has(k) or b[k] != a[k]:
			return false
	return true


func _check(ok: bool, what: String, text := "") -> void:
	if ok:
		return
	fail_count += 1
	printerr("MISMATCH: %s for input: %s" % [what, text.left(60)])


func _bench(inputs: Array) -> void:
	for mode in [GDStringMap.Mode.FULL, GDStringMap.Mode.STRING]:
		var mode_name := "FULL" if mode == GDStringMap.Mode.FULL else "STRING"
		print("\n=== mode=%s iterations=%d ===" % [mode_name, iterations])
		for text in inputs:
			var label: String = text.left(40).replace("\n", "\\n")
			if text.length() > 100:
				label = "%s... (%d chars)" % [text.left(30).replace("\n", "\\n"), text.length()]
			var gd_stats := _time_one(gd_factory, text, mode)
			var cpp_stats := _time_one(cpp_factory, text, mode)
			var speedup: float = gd_stats.avg / cpp_stats.avg if cpp_stats.avg > 0 else 0.0
			print("  [%s]\n    gdscript: avg %.1f us  min %.1f us\n    cpp:      avg %.1f us  min %.1f us  (%.1fx)" % [
				label, gd_stats.avg, gd_stats.min, cpp_stats.avg, cpp_stats.min, speedup,
			])


func _time_one(factory: Callable, text: String, mode: int) -> Dictionary:
	for i in warmup:
		var sm = factory.call(text, mode)
	var total := 0
	var min_us := 9223372036854775807
	for i in iterations:
		var t0 := Time.get_ticks_usec()
		var sm = factory.call(text, mode)
		var elapsed := Time.get_ticks_usec() - t0
		total += elapsed
		min_us = mini(min_us, elapsed)
	return { "avg": float(total) / iterations, "min": float(min_us) }
