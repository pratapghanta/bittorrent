#if !defined(METAINFO_HPP)
#define METAINFO_HPP

#include <string>
#include <vector>   
#include <map>
#include <memory>
#include <variant>

namespace BT {
    /* TODO: Better name for MI_Object_t */
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
        
        MI_Object_t(MI_Int_t const v) : mValue(v) {}
        MI_Object_t(MI_String_t const& v) : mValue(v) {}
        MI_Object_t(MI_ListPtr_t const& v) : mValue(v) {}
        MI_Object_t(MI_DictPtr_t const& v) : mValue(v) {}

        MI_Object_t(MI_Object_t const&) = default;
        MI_Object_t& operator=(MI_Object_t const&) = default;

        MI_Object_t(MI_Object_t&&) = default;
        MI_Object_t& operator=(MI_Object_t&&) = default;
        
        ~MI_Object_t() = default;

		MI_Int_t GetInt() { return std::get<MI_Int_t>(mValue); }
		MI_String_t GetString() { return std::get<MI_String_t>(mValue); }
		MI_ListPtr_t GetListPtr() { return std::get<MI_ListPtr_t>(mValue); }
		MI_DictPtr_t GetDictPtr() { return std::get<MI_DictPtr_t>(mValue); }

		void SetInt(MI_Int_t const v) { mValue = v; }
        void SetString(MI_String_t const& v) { mValue = v; }
        void SetListPtr(MI_ListPtr_t const& v) { mValue = v; }
        void SetDictPtr(MI_DictPtr_t const& v) { mValue = v; }

        private:
        std::variant<std::monostate,
                     MI_Int_t, 
                     MI_String_t, 
                     MI_ListPtr_t, 
                     MI_DictPtr_t> mValue;
    };

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
