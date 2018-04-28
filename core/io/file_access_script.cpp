#include "file_access_script.h"
#include "script_language.h"

void PackSourceScript::add_path(const String &file_name) {
	ERR_FAIL_COND(!get_script_instance());

	// The data in PackedFile seems to be fully specific for FileAccessPack.
	// We basically give it dummy values.
	// "ofs" at 0 causes PackedData to think the file was "erased"?
	// So we use 1 instead of 0.
	uint8_t dummy[16];
	PackedData::get_singleton()->add_path("", file_name, 1, 1, dummy, pack_source);
}

FileAccess *PackSourceScript::get_file(const String &file_name) {
	ERR_FAIL_NULL_V(get_script_instance(), NULL);

	Variant v = file_name;
	const Variant *p = &v;
	Variant::CallError ce;
	Variant ret = get_script_instance()->call("open_file", &p, 1, ce);
	if (ce.error != Variant::CallError::CALL_OK || ret.get_type() == Variant::Type::NIL) {
		return NULL;
	} else {
		Ref<FileAccessScript> a(ret);
		ERR_FAIL_NULL_V(a->get_script_instance(), NULL);
		FileAccessScriptInternal *f = memnew(FileAccessScriptInternal(a.ptr()));
		return f;
	}
}

void PackSourceScript::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_path", "file_name"), &PackSourceScript::add_path);

	BIND_VMETHOD(MethodInfo(Variant::OBJECT, "read_file", PropertyInfo(Variant::STRING, "file_name")));
}

void PackSourceScriptInternal::disable() {
	disabled = true;
}

bool PackSourceScriptInternal::try_open_pack(const String &p_path) {
	return false;
}

FileAccess *PackSourceScriptInternal::get_file(const String &p_path, PackedData::PackedFile *p_file) {
	if (disabled) {
		return NULL;
	}

	return parent->get_file(p_path);
}

PackSourceScriptInternal::PackSourceScriptInternal(PackSourceScript *theparent) {
	parent = theparent;
}

PackSourceScript::PackSourceScript() {
	pack_source = memnew(PackSourceScriptInternal(this));

	PackedData::get_singleton()->add_pack_source(pack_source);
}

PackSourceScript::~PackSourceScript() {
	WARN_PRINT("Releasing of PackSourceScript is not fully implemented yet.");
	pack_source->disable();
}

void FileAccessScript::_bind_methods() {
	BIND_VMETHOD(MethodInfo("close"));

	BIND_VMETHOD(MethodInfo("seek", PropertyInfo(Variant::INT, "p_position")));
	BIND_VMETHOD(MethodInfo("seek_end", PropertyInfo(Variant::INT, "p_position")));

	BIND_VMETHOD(MethodInfo(Variant::INT, "get_position"));
	BIND_VMETHOD(MethodInfo(Variant::INT, "get_len"));
	BIND_VMETHOD(MethodInfo(Variant::BOOL, "eof_reached"));

	BIND_VMETHOD(MethodInfo(Variant::INT, "get_8"));
	BIND_VMETHOD(MethodInfo(Variant::INT, "get_16"));
	BIND_VMETHOD(MethodInfo(Variant::INT, "get_32"));
	BIND_VMETHOD(MethodInfo(Variant::INT, "get_64"));

	BIND_VMETHOD(MethodInfo(Variant::POOL_BYTE_ARRAY, "get_buffer", PropertyInfo(Variant::INT, "size")));

	BIND_VMETHOD(MethodInfo(Variant::INT, "get_error"));
}

Error FileAccessScriptInternal::_open(const String &p_path, int p_mode_flags) {
	ERR_FAIL_V(ERR_UNAVAILABLE);
	return ERR_UNAVAILABLE;
}

uint64_t FileAccessScriptInternal::_get_modified_time(const String &p_file) {
	return 0;
}

void FileAccessScriptInternal::close() {
	if (!is_open()) {
		return;
	}

	// REALLY shouldn't happen but I can't do much about it.
	if (parent->get_script_instance()) {
		Variant::CallError ce;
		parent->get_script_instance()->call("close", NULL, 0, ce);
	} else {
		WARN_PRINT("FileAccessScript has no script instance in close()!");
	}

	parent = Ref<FileAccessScript>(NULL);
}

bool FileAccessScriptInternal::is_open() const {
	return (parent.is_valid());
}

void FileAccessScriptInternal::seek(size_t p_position) {
	ERR_FAIL_NULL(parent->get_script_instance());
	Variant v = (uint64_t)p_position;
	const Variant *p = &v;
	Variant::CallError ce;
	parent->get_script_instance()->call("seek", &p, 1, ce);
}

void FileAccessScriptInternal::seek_end(int64_t p_position) {
	ERR_FAIL_NULL(parent->get_script_instance());
	Variant v = (uint64_t)p_position;
	const Variant *p = &v;
	Variant::CallError ce;
	parent->get_script_instance()->call("seek_end", &p, 1, ce);
}

size_t FileAccessScriptInternal::get_position() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);
	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_position", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, 0);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, 0);

	return (uint64_t)ret;
}

size_t FileAccessScriptInternal::get_len() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);
	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_len", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, 0);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, 0);

	return (uint64_t)ret;
}

bool FileAccessScriptInternal::eof_reached() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), false);
	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("eof_reached", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, false);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::BOOL, false);

	return (bool)ret;
}

uint8_t FileAccessScriptInternal::get_8() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);
	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_8", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, false);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, false);

	uint64_t val = ret;
	ERR_FAIL_COND_V(val < 0 || val > 255, 0);
	return (uint8_t)ret;
}

uint16_t FileAccessScriptInternal::get_16() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);

	if (!parent->get_script_instance()->has_method("get_16")) {
		return FileAccess::get_16();
	}

	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_16", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, false);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, false);

	uint64_t val = ret;
	ERR_FAIL_COND_V(val < 0 || val > 65535, 0);
	return (uint16_t)ret;
}

uint32_t FileAccessScriptInternal::get_32() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);

	if (!parent->get_script_instance()->has_method("get_32")) {
		return FileAccess::get_32();
	}

	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_32", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, false);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, false);

	uint64_t val = ret;
	ERR_FAIL_COND_V(val < 0 || val > 4294967295, 0);
	return (uint32_t)ret;
}

uint64_t FileAccessScriptInternal::get_64() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);

	if (!parent->get_script_instance()->has_method("get_64")) {
		return FileAccess::get_64();
	}

	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_64", NULL, 0, ce);

	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, false);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, false);

	return (uint64_t)ret;
}

int FileAccessScriptInternal::get_buffer(uint8_t *p_dst, int p_length) const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), 0);

	if (!parent->get_script_instance()->has_method("get_buffer")) {
		return FileAccess::get_buffer(p_dst, p_length);
	}

	Variant v = p_length;
	const Variant *p = &v;

	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_buffer", &p, 1, ce);
	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, 0);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, 0);

	PoolByteArray val = ret;
	ERR_FAIL_COND_V(val.size() > p_length, 0);

	PoolByteArray::Read read = val.read();
	memcpy(p_dst, read.ptr(), val.size());

	return val.size();
}

Error FileAccessScriptInternal::get_error() const {
	ERR_FAIL_NULL_V(parent->get_script_instance(), FAILED);

	Variant::CallError ce;
	Variant ret = parent->get_script_instance()->call("get_error", NULL, 0, ce);
	ERR_FAIL_COND_V(ce.error != Variant::CallError::CALL_OK, FAILED);
	ERR_FAIL_COND_V(ret.get_type() != Variant::Type::INT, FAILED);
	return (Error)(uint64_t)ret;
}

void FileAccessScriptInternal::flush() {
	ERR_FAIL();
}

void FileAccessScriptInternal::store_8(uint8_t p_dest) {
	ERR_FAIL();
}

bool FileAccessScriptInternal::file_exists(const String &p_name) {
	return false;
}

FileAccessScriptInternal::FileAccessScriptInternal(const Ref<FileAccessScript> &newparent) {
	parent = newparent;
}

FileAccessScriptInternal::~FileAccessScriptInternal() {
	close();
}

/*
	if (get_script_instance()) {
		Variant v = filename;
		const Variant *p = &v;
		Variant::CallError ce;
		Variant ret = get_script_instance()->call("read_file", &p, 1, ce);
		if (ce.error != Variant::CallError::CALL_OK || ret.get_type() == Variant::Type::NIL) {
			print_line("shit");
		} else {
			Ref<FileAccessScript> a(ret);
			a->work_pls();
		}
	}
*/
