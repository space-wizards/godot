#include "file_access_pack.h"
#include "os/file_access.h"
#include "reference.h"

class PackSourceScriptInternal;

class PackSourceScript : public Reference {
	GDCLASS(PackSourceScript, Reference);

	static void _bind_methods();

	PackSourceScriptInternal *pack_source;

public:
	void add_path(const String &file_name);

	FileAccess *get_file(const String &p_path);

	PackSourceScript();
	~PackSourceScript();
};

class PackSourceScriptInternal : public PackSource {
	PackSourceScript *parent;
	bool disabled = false;

	friend class PackSourceScript;

	PackSourceScriptInternal(PackSourceScript *theparent);

public:
	// TEMPORARY: can't cleanly remove pack sources atm.
	void disable();
	virtual bool try_open_pack(const String &p_path);
	virtual FileAccess *get_file(const String &p_path, PackedData::PackedFile *p_file);
};

class FileAccessScript : public Reference {
	GDCLASS(FileAccessScript, Reference);

	static void _bind_methods();

	friend class FileAccessScriptInternal;
};

class FileAccessScriptInternal : public FileAccess {
	Ref<FileAccessScript> parent;

	bool open;

protected:
	Error _open(const String &p_path, int p_mode_flags);

public:
	virtual uint64_t _get_modified_time(const String &p_file);
	virtual void close();
	virtual bool is_open() const;

	virtual void seek(size_t p_position);
	virtual void seek_end(int64_t p_position = 0);

	virtual size_t get_position() const;
	virtual size_t get_len() const;
	virtual bool eof_reached() const;

	virtual uint8_t get_8() const;
	virtual uint16_t get_16() const;
	virtual uint32_t get_32() const;
	virtual uint64_t get_64() const;

	virtual int get_buffer(uint8_t *p_dst, int p_length) const;

	virtual Error get_error() const;

	// Things that need to be implemented but are not supported.
	virtual void flush();
	virtual void store_8(uint8_t p_dest);
	virtual bool file_exists(const String &p_name);

	FileAccessScriptInternal(const Ref<FileAccessScript> &newparent);
	~FileAccessScriptInternal();
};
