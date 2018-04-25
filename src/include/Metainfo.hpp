#ifndef METAINFO_HPP
#define METAINFO_HPP

#include <map>
#include <vector>
#include <string>	
#include <memory>

namespace BT {
	struct MI_Object_t;

	using MI_Int_t = long;
	using MI_String_t = std::string;
	using MI_List_t = std::vector<MI_Object_t>;
	using MI_Dict_t = std::map<MI_String_t, MI_Object_t>;

	using MI_ListPtr_t = std::shared_ptr<MI_List_t>;
	using MI_DictPtr_t = std::shared_ptr<MI_Dict_t>;

	enum class MI_ObjectKind_t : short {
		MI_INT,
		MI_STRING,
		MI_LIST,
		MI_DICT,

		MI_END
	};

	/* TODO: Use union for effectively using space. */
	struct MI_ObjectValue_t {
		MI_Int_t nInt;
		MI_String_t cStr;
		MI_ListPtr_t vList;
		MI_DictPtr_t mDict;
	};

	struct MI_Object_t {
		MI_ObjectKind_t kind;
		MI_ObjectValue_t value;
	};

	class Metainfo {
	public:
		Metainfo(MI_DictPtr_t const i, std::string const& h) : data(i), infoHash(h) {}
		MI_DictPtr_t getData() const { return data;  }
		std::string const getInfoHash() const { return infoHash; }

	private:
		MI_DictPtr_t data;
		std::string infoHash;
	};
}
#endif
