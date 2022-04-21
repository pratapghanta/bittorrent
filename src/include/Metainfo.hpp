#if !defined(METAINFO_HPP)
#define METAINFO_HPP

#include <string>
#include <vector>	
#include <map>
#include <memory>
#include <variant>

namespace BT {
	class MI_Object_t;

	using MI_Int_t = long;
	using MI_String_t = std::string;
	using MI_List_t = std::vector<MI_Object_t>;
	using MI_Dict_t = std::map<MI_String_t, MI_Object_t>;

	using MI_ListPtr_t = std::shared_ptr<MI_List_t>;
	using MI_DictPtr_t = std::shared_ptr<MI_Dict_t>;

	/* Wrapper needed to break cyclic dependency using forward declaration */
	class MI_Object_t {
	public:
		MI_Object_t() = default;
		
		MI_Object_t(MI_Object_t const&) = default;
		MI_Object_t& operator=(MI_Object_t const&) = default;

		MI_Object_t(MI_Object_t&&) = default;
		MI_Object_t& operator=(MI_Object_t&&) = default;
		
		~MI_Object_t() = default;

		template<typename T>
		void Set(T const& v) {
			mValue = v;
		}

		template<typename T>
		T Get() const {
			return std::get<T>(mValue);
		}

		private:
		std::variant<std::monostate,
		             MI_Int_t, 
					 MI_String_t, 
					 MI_ListPtr_t, 
					 MI_DictPtr_t> mValue;
	};

	template void MI_Object_t::Set<MI_Int_t>(MI_Int_t const&);
	template void MI_Object_t::Set<MI_String_t>(MI_String_t const&);
	template void MI_Object_t::Set<MI_ListPtr_t>(MI_ListPtr_t const&);
	template void MI_Object_t::Set<MI_DictPtr_t>(MI_DictPtr_t const&);

	template MI_Int_t MI_Object_t::Get<MI_Int_t>() const;
	template MI_String_t MI_Object_t::Get<MI_String_t>() const;
	template MI_ListPtr_t MI_Object_t::Get<MI_ListPtr_t>() const;
	template MI_DictPtr_t MI_Object_t::Get<MI_DictPtr_t>() const;

	struct Metainfo_t {
		MI_Dict_t mData;
		std::string  mInfoHash;
	
		void Reset() {
			mData.clear();
			mInfoHash.clear();
		}
	};
}
#endif // !defined(METAINFO_HPP)
