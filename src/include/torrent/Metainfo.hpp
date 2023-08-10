#if !defined(METAINFO_HPP)
#define METAINFO_HPP

#include <string>
#include <vector>   
#include <unordered_map>
#include <memory>
#include <variant>

namespace BT 
{
    /* TODO: Better name for MI_Object */
    class MI_Object;

    using MI_Int = long;
    using MI_String = std::string;
    using MI_List = std::vector<MI_Object>;
    using MI_Dict = std::unordered_map<MI_String, MI_Object>;
    using MI_ListPtr = std::shared_ptr<MI_List>;
    using MI_DictPtr = std::shared_ptr<MI_Dict>;

    /* Wrapper needed to break cyclic dependency using forward declaration */
    class MI_Object 
    {
    public:
        MI_Object() = default;
        
        MI_Object(MI_Int const v) : mValue(v) {}
        MI_Object(MI_String const& v) : mValue(v) {}
        MI_Object(MI_ListPtr const& v) : mValue(v) {}
        MI_Object(MI_DictPtr const& v) : mValue(v) {}

        MI_Object(MI_Object const&) = default;
        MI_Object& operator=(MI_Object const&) = default;

        MI_Object(MI_Object&&) = default;
        MI_Object& operator=(MI_Object&&) = default;
        
        ~MI_Object() = default;

		MI_Int GetInt() { return std::get<MI_Int>(mValue); }
		MI_String GetString() { return std::get<MI_String>(mValue); }
		MI_ListPtr GetListPtr() { return std::get<MI_ListPtr>(mValue); }
		MI_DictPtr GetDictPtr() { return std::get<MI_DictPtr>(mValue); }

		void SetInt(MI_Int const v) { mValue = v; }
        void SetString(MI_String const& v) { mValue = v; }
        void SetListPtr(MI_ListPtr const& v) { mValue = v; }
        void SetDictPtr(MI_DictPtr const& v) { mValue = v; }

        private:
        std::variant<std::monostate,
                     MI_Int, 
                     MI_String, 
                     MI_ListPtr, 
                     MI_DictPtr> mValue;
    };

    struct Metainfo 
    {
        void Reset() 
        {
            mData.clear();
            mInfoHash.clear();
        }

        MI_Dict mData;
        std::string mInfoHash;
    };
}

#endif // !defined(METAINFO_HPP)
