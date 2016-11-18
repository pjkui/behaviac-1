/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tencent is pleased to support the open source community by making behaviac available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at http://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BEHAVIAC_COMMON_OBJECT_TAGOBJECT_H_
#define _BEHAVIAC_COMMON_OBJECT_TAGOBJECT_H_

#include "behaviac/common/base.h"
#include "behaviac/common/serialization/ionode.h"
#include "behaviac/common/swapbyte.h"
#include "behaviac/common/serialization/textnode.h"

#if BEHAVIAC_CCDEFINE_MSVC
//C4189: 'ms_members' : local variable is initialized but not referenced
#pragma warning (disable : 4189 )
#endif//#if BEHAVIAC_CCDEFINE_MSVC

namespace behaviac
{
	class IMemberBase;
	class CTagObject;

	template<class Parent> void LuaCopyFunctionTable(int tagDest);

	class BEHAVIAC_API CTagTypeDescriptor : public CRTTIBase
	{
	public:
		BEHAVIAC_DECLARE_ROOT_DYNAMIC_TYPE(CTagTypeDescriptor, CRTTIBase);

		typedef behaviac::map<const char*, const class CTagTypeDescriptor*> TypesMap_t;
	};

	// CTagObjectDescriptor
	class BEHAVIAC_API CTagObjectDescriptor : public CTagTypeDescriptor
	{
	public:
		BEHAVIAC_DECLARE_DYNAMIC_TYPE(CTagObjectDescriptor, CTagTypeDescriptor);

		typedef behaviac::vector<behaviac::IMemberBase*> MembersVector_t;
		typedef behaviac::map<behaviac::CStringCRC, behaviac::IMemberBase*> MembersMap_t;

		struct MembersContainer
		{
			MembersMap_t		membersMap;
			MembersVector_t		membersVector;
		};

		CTagObjectDescriptor() : m_parent(0), ms_flags(0), ms_isInitialized(false), m_isRefType(false)
		{}

		void Load(behaviac::CTagObject* parent, const behaviac::IIONode* node) const;
		void Save(const behaviac::CTagObject* parent, behaviac::IIONode* node) const;

		void LoadState(behaviac::CTagObject* parent, const behaviac::IIONode* node) const;
		void SaveState(const behaviac::CTagObject* parent, behaviac::IIONode* node) const;

		const behaviac::IMemberBase* GetMember(const behaviac::CStringCRC& propertyId) const;
#if BEHAVIAC_ENABLE_NETWORKD
		void ReplicateProperties(behaviac::CTagObject* parent);
#endif

		static void PushBackMember(MembersContainer& inMembers, behaviac::IMemberBase* toAddMember);

		CTagObjectDescriptor& Get()
		{
			return *this;
		}

		CTagObjectDescriptor*	m_parent;
		MembersContainer		ms_members;

		behaviac::wstring		m_displayName;
		behaviac::wstring		m_desc;

		uint32_t				ms_flags;
		bool					ms_isInitialized;
		bool					m_isRefType;
	};

	// CTagObjectDescriptorBSS
	class BEHAVIAC_API CTagObjectDescriptorBSS
	{
		CTagObjectDescriptor* m_descriptor;
	public:
		CTagObjectDescriptorBSS() : m_descriptor(0)
		{}

		CTagObjectDescriptor& Get()
		{
			if (!m_descriptor)
			{
				m_descriptor = BEHAVIAC_NEW CTagObjectDescriptor;
			}

			BEHAVIAC_ASSERT(m_descriptor);

			return *m_descriptor;
		}

		void Cleanup();
	};

	// behaviac::CTagObject
	class BEHAVIAC_API CTagObject : public CRTTIBase
	{
	public:
		typedef CTagObjectDescriptor::MembersContainer MembersContainer;

		BEHAVIAC_DECLARE_MEMORY_OPERATORS(behaviac::CTagObject);
		BEHAVIAC_DECLARE_ROOT_DYNAMIC_TYPE(behaviac::CTagObject, CRTTIBase);

		void LoadFromXML(const behaviac::XmlConstNodeRef& xmlNode);
		void SaveToXML(const behaviac::XmlNodeRef& xmlNode);

		void Load(const behaviac::IIONode* node);
		void Save(behaviac::IIONode* node) const;

		void LoadState(const behaviac::IIONode* node)
		{
			GetDescriptor().LoadState(this, node);
		}
		void SaveState(behaviac::IIONode* node) const
		{
			GetDescriptor().SaveState(this, node);
		}

		// NOTE: before T::GetDescriptor() returns, it must make sure T::RegisterProperties() has been called
		virtual const CTagObjectDescriptor& GetDescriptor() const = 0;

		static void RegisterProperties() { }

		// Scripting
		static int GetScriptTableId()
		{
			return -1;
		}
		static void CopyScriptProperties(int tagDest)
		{
			BEHAVIAC_UNUSED_VAR(tagDest);
		}
		static bool IsAScriptKindOf(int tag)
		{
			BEHAVIAC_UNUSED_VAR(tag);
			return false;
		}
		static CTagObjectDescriptor& GetObjectDescriptor();
		static CTagObjectDescriptor& GetObjectDescriptorDirectly();
		static void CleanupObjectDescriptor();
	protected:
		static CTagObjectDescriptorBSS ms_descriptor;
	};


}//namespace behaviac

namespace behaviac
{
    namespace StringUtils
    {
        BEHAVIAC_API behaviac::XmlNodeRef MakeXmlNodeStruct(const char* str, const behaviac::string& typeNameT);

        template<typename T>
        inline bool FromString_Struct(const char* str, T& val)
        {
			char temp[1024];
			string_sprintf(temp, "%s", GetClassTypeName((T*)0));

			behaviac::XmlNodeRef xmlNode = MakeXmlNodeStruct(str, temp);

            if ((IXmlNode*)xmlNode)
            {
                CTextNode textNode(xmlNode);

                val.Load(&textNode);

                return true;
            }

            return false;
        }

        BEHAVIAC_API bool MakeStringFromXmlNodeStruct(behaviac::XmlConstNodeRef xmlNode, behaviac::string& result);

        template<typename T>
        inline behaviac::string ToString_Struct(T& val)
        {
            //behaviac::string typeNameT = FormatString("%s", GetClassTypeName((T*)0));
            const char* typeNameT = GetClassTypeName((T*)0);

            behaviac::XmlNodeRef xmlNode = CreateXmlNode(typeNameT);

            CTextNode textNode(xmlNode);

            val.Save(&textNode);

            behaviac::string result;

            if (MakeStringFromXmlNodeStruct(xmlNode, result))
            {
                return result;
            }

            return "";
        }
    }

	BEHAVIAC_API bool Equal_Struct(const CTagObjectDescriptor& object_desc, const behaviac::CTagObject* lhs, const behaviac::CTagObject* rhs);
}

// behaviac::CTagObject macros
#define ACCESS_PROPERTY_METHOD													\
    template<typename T, typename R>											\
    BEHAVIAC_FORCEINLINE R& _Get_Property_();									\
																				\
    template<typename T, typename R>											\
    BEHAVIAC_FORCEINLINE R _Execute_Method_();									\
    template<typename T, typename R, typename P0>								\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0);								\
    template<typename T, typename R, typename P0, typename P1>					\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1);							\
    template<typename T, typename R, typename P0, typename P1, typename P2>		\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2);																									\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3>																	\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3);																							\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3, typename P4>														\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3, P4);																						\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>											\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3, P4, P5);																					\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>								\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3, P4, P5, P6);																				\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>				\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3, P4, P5, P6, P7);																		\
    template<typename T, typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>	\
    BEHAVIAC_FORCEINLINE R _Execute_Method_(P0, P1, P2, P3, P4, P5, P6, P7, P8);


/////////////////////////////////////////////////////////////////////////////////////////
/**
classFullNameWithNamespace is the class full name with namespace, like test_ns::AgentTest

even the class is delared in a namespace, it is still advised to use the full name with the name space.
*/
#define BEHAVIAC_DECLARE_AGENTTYPE(classFullNameWithNamespace, parentClassName)		\
    BEHAVIAC_DECLARE_MEMORY_OPERATORS_AGENT(classFullNameWithNamespace)					\
    BEHAVIAC_DECLARE_ROOT_DYNAMIC_TYPE(classFullNameWithNamespace, parentClassName)		\
    virtual const behaviac::CTagObjectDescriptor& GetDescriptor() const					\
    { return classFullNameWithNamespace::GetObjectDescriptor(); }						\
	ACCESS_PROPERTY_METHOD

/////////////////////////////////////////////////////////////////////////////////////////

namespace behaviac
{
	typedef behaviac::map<behaviac::string, const CTagObjectDescriptorBSS*>	TagObjectDescriptorMap_t;
	BEHAVIAC_API TagObjectDescriptorMap_t& GetTagObjectDescriptorMaps();
	BEHAVIAC_API void CleanupTagObjectDescriptorMaps();

	template <typename T>
	inline CTagObjectDescriptor& GetObjectDescriptor()
	{
		static CTagObjectDescriptor s_temp;
		return s_temp;
	}

	template <typename T>
	inline void RegisterProperties()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T, bool bManagedType>
	struct ObjectDescriptorGettter
	{
		static CTagObjectDescriptor& _GetObjectDescriptor()
		{
			return GetObjectDescriptor<T>();
		}
	};

	template <typename T, bool bManagedType>
	struct RegisterPropertiesGetter
	{
		static void _RegisterProperties()
		{
			RegisterProperties<T>();
		}
	};

	template <typename T>
	struct ObjectDescriptorGettter<T, true>
	{
		static CTagObjectDescriptor& _GetObjectDescriptor()
		{
			return T::GetObjectDescriptor();
		}
	};

	template <typename T>
	struct RegisterPropertiesGetter<T, true>
	{
		static void _RegisterProperties()
		{
			T::RegisterProperties();
		}
	};
}//

#define BEHAVIAC_BEGIN_STRUCT(className)										\
    behaviac::CTagObjectDescriptorBSS className::ms_descriptor;					\
    behaviac::CTagObjectDescriptor& className::GetObjectDescriptorDirectly(){return className::ms_descriptor.Get();}\
    behaviac::CTagObjectDescriptor& className::GetObjectDescriptor()			\
    {																			\
        className::RegisterProperties();										\
        return className::ms_descriptor.Get();									\
    }																			\
    void className::RegisterProperties()                                        \
    {                                                                           \
        if (className::GetObjectDescriptorDirectly().ms_isInitialized)			\
            return;                                                             \
        behaviac::TagObjectDescriptorMap_t& maps = behaviac::GetTagObjectDescriptorMaps();\
        const char* classNameStr = className::GetClassTypeName();				\
        behaviac::TagObjectDescriptorMap_t::iterator it = maps.find(classNameStr);\
        if (it != maps.end()) {													\
            BEHAVIAC_LOGWARNING("%s duplated in BEHAVIAC_BEGIN_STRUCT!\n", classNameStr);\
		        }																\
        maps[classNameStr] = &className::ms_descriptor;							\
        RegisterParent();                                                       \
        behaviac::CTagObjectDescriptor::MembersContainer& ms_members = className::GetObjectDescriptorDirectly().ms_members;\
        BEHAVIAC_UNUSED_VAR(ms_members);                                        \
        typedef className objectType;                                           \
        objectType::GetObjectDescriptorDirectly().ms_isInitialized = true;		\
        objectType::GetObjectDescriptorDirectly().m_isRefType = className::ms_bIsRefType;

/////////////////////////////////////////////////////////////////////////////////////////
#define BEHAVIAC_END_STRUCT()         }

//deparated, to use BEHAVIAC_STRUCT_DISPLAY_INFO
#define BEHAVIAC_STRUCT_DISPLAYNAME(displayName)    objectType::GetObjectDescriptorDirectly().m_displayName = displayName;
#define BEHAVIAC_STRUCT_DESC(desc)    objectType::GetObjectDescriptorDirectly().m_desc = desc;

#define BEHAVIAC_STRUCT_DISPLAY_INFO(displayName, desc)							\
    objectType::GetObjectDescriptorDirectly().m_displayName = displayName;		\
    if (desc) {objectType::GetObjectDescriptorDirectly().m_desc = desc;}		\
	    else {objectType::GetObjectDescriptorDirectly().m_desc = displayName;}

////////////////////////////////////////////////////////////////////////////////
// No-virtual behaviac::CTagObject declaration
////////////////////////////////////////////////////////////////////////////////
#define DECLARE_BEHAVIAC_OBJECT_BASE(className, bRefType)	\
    protected:                                                                  \
    static behaviac::CTagObjectDescriptorBSS ms_descriptor;						\
    public:                                                                     \
	const static bool ms_bIsRefType = bRefType;									\
    static behaviac::CTagObjectDescriptor& GetObjectDescriptor();				\
    static behaviac::CTagObjectDescriptor& GetObjectDescriptorDirectly();		\
    void Load(const behaviac::IIONode* node)									\
    {                                                                           \
        RegisterProperties();                                                   \
        className::GetObjectDescriptorDirectly().Load((behaviac::CTagObject*)this, node); \
    }                                                                           \
    void Save(behaviac::IIONode* node) const									\
    {                                                                           \
        RegisterProperties();                                                   \
        className::GetObjectDescriptorDirectly().Save((behaviac::CTagObject*)this, node); \
    }                                                                           \
																				\
    void LoadState(const behaviac::IIONode* node)								\
    {                                                                           \
        RegisterProperties();                                                   \
        className::GetObjectDescriptorDirectly().LoadState((behaviac::CTagObject*)this, node);    \
    }                                                                           \
    void SaveState(behaviac::IIONode* node) const								\
    {                                                                           \
        RegisterProperties();                                                   \
        className::GetObjectDescriptorDirectly().SaveState((behaviac::CTagObject*)this, node);    \
    }                                                                           \
																				\
    static void RegisterParent() {}                                             \
	static void RegisterProperties();                                           

#define DECLARE_BEHAVIAC_STRUCT_BASE_(className, bRefType)	\
    DECLARE_BEHAVIAC_OBJECT_BASE(className, bRefType)						\
    bool _Object_Equal_(const className& rhs)	const						\
    {																		\
        const behaviac::CTagObjectDescriptor& object_desc = className::GetObjectDescriptor();	\
        return behaviac::Equal_Struct(object_desc, (const behaviac::CTagObject*)this, (const behaviac::CTagObject*)&rhs); \
    }																		\
    bool ParseString(const char* str)										\
    {																		\
        return behaviac::StringUtils::FromString_Struct(str, *this);		\
    }																		\
    behaviac::string ToString() const										\
    {																		\
        return behaviac::StringUtils::ToString_Struct(*this);				\
    }																		

/////////////////////////////////////////////////////////////////////////////////////////
/**
classFullNameWithNamespace is the class full name with namespace, like test_ns::AgentTest,

even the class is delared in a namespace, it is still advised to use the full name with the name space.
the corresponding BEHAVIAC_BEGIN_STRUCT/BEHAVIAC_END_STRUCT in the cpp can be put in or out of that namespace.
*/
#define DECLARE_BEHAVIAC_OBJECT_STRUCT_V2(classFullNameWithNamespace, bRefType)			\
    DECLARE_BEHAVIAC_STRUCT_BASE_(classFullNameWithNamespace, bRefType)					\
    static bool IsOfMyKind(const behaviac::CTagObject*)									\
    { return true; }																	\
    static const char* GetClassTypeName()												\
    { return #classFullNameWithNamespace; }

#define DECLARE_BEHAVIAC_OBJECT_STRUCT_V1(classFullNameWithNamespace) DECLARE_BEHAVIAC_OBJECT_STRUCT_V2(classFullNameWithNamespace, false)

/**
DECLARE_BEHAVIAC_STRUCT can accept 1 or 2 parameters

the 1st param is the full class name with the namespace if any, like test_ns::AgentTest, 
even the class is delared in a namespace, it is still advised to use the full name with the name space.
the corresponding BEHAVIAC_BEGIN_STRUCT/BEHAVIAC_END_STRUCT in the cpp can be put in or out of that namespace.

the 2nd param is true or false indicating if the class is a ref type. a ref type is used as a pointer.
*/
#define DECLARE_BEHAVIAC_STRUCT(...) _BEHAVIAC_ARGUMENT_SELECTOR2_((__VA_ARGS__, DECLARE_BEHAVIAC_OBJECT_STRUCT_V2, DECLARE_BEHAVIAC_OBJECT_STRUCT_V1))(__VA_ARGS__)
#define _BEHAVIAC_ARGUMENT_SELECTOR2_(__args) _BEHAVIAC_GET_2TH_ARGUMENT_ __args
#define _BEHAVIAC_GET_2TH_ARGUMENT_(__p1,__p2,__n,...) __n

/**
ex: BEHAVIAC_EXTEND_EXISTING_TYPE(myNode, cocos2d::Node)
*/
#define BEHAVIAC_EXTEND_EXISTING_TYPE(myType, existingType)				\
	BEHAVIAC_DECLARE_SPECIALIZE_TYPE_HANDLER(existingType);				\
	namespace behaviac													\
	{																	\
		template <>														\
		inline CTagObjectDescriptor& GetObjectDescriptor<existingType>()\
		{																\
			return myType::GetObjectDescriptor();						\
		}																\
		template <>														\
		inline void RegisterProperties<existingType>()					\
		{																\
			myType::RegisterProperties();								\
		}																\
		namespace Meta													\
		{																\
			template <>													\
			struct TypeMapper<existingType>								\
			{															\
				typedef myType Type;									\
			};															\
		}																\
	}

/**
BEHAVIAC_DECLARE_OBJECT is only used for IList and System::Object,
in general, please use DECLARE_BEHAVIAC_STRUCT, unless you know what you are doing
*/
#define BEHAVIAC_DECLARE_OBJECT(classFullNameWithNamespace)					\
    DECLARE_BEHAVIAC_OBJECT_BASE(classFullNameWithNamespace, false)			\
    static bool IsOfMyKind(const behaviac::CTagObject*)						\
    { return true; }														\
    static const char* GetClassTypeName()									\
    { return #classFullNameWithNamespace; }

namespace behaviac
{
	/////////////////////////////////////////////////////////////////////////////////////////
	struct EnumValueItem_t
	{
		behaviac::string		nativeName;
		behaviac::string		name;
		behaviac::wstring		m_displayName;
		behaviac::wstring		m_desc;

		EnumValueItem_t& DISPLAYNAME(const wchar_t* _displayName)
		{
			m_displayName = _displayName;

			return *this;
		}

		EnumValueItem_t& DESC(const wchar_t* _desc)
		{
			m_desc = _desc;

			return *this;
		}

		EnumValueItem_t& DISPLAY_INFO(const wchar_t* _displayName, const wchar_t* _desc = 0)
		{
			m_displayName = _displayName;

			if (_desc)
			{
				m_desc = _desc;

			}
			else
			{
				m_desc = m_displayName;
			}

			return *this;
		}
	};

	typedef behaviac::map<uint32_t, EnumValueItem_t>	EnumValueNameMap_t;

	class EnumClassDescription_t : public CTagTypeDescriptor
	{
	public:
		BEHAVIAC_DECLARE_DYNAMIC_TYPE(EnumClassDescription_t, CTagTypeDescriptor);

		EnumValueNameMap_t		valueMaps;
		behaviac::wstring		m_displayName;
		behaviac::wstring		m_desc;
	};

	//template<typename T>
	//inline const EnumClassDescription_t& GetEnumClassValueNames(T*p)
	//{
	//	BEHAVIAC_ASSERT(0);
	//    return *((EnumClassDescription_t*)0);
	//}
	template<typename T>
	const EnumClassDescription_t& GetEnumClassValueNames(T* p);

	template<typename T>
	inline behaviac::string EnumValueToString(const T& v)
	{
		const EnumClassDescription_t& ecd = behaviac::GetEnumClassValueNames((T*)0);

		for (EnumValueNameMap_t::const_iterator it = ecd.valueMaps.begin(); it != ecd.valueMaps.end(); ++it)
		{
			const EnumValueItem_t& valueItem = it->second;

			if (((T)it->first) == v)
			{
				return valueItem.name;
			}
		}

		return "NotAnEnum";
	}

	template<typename T>
	inline bool EnumValueFromString(const char* valueStr, T& v)
	{
		const EnumClassDescription_t& ecd = behaviac::GetEnumClassValueNames((T*)0);

		for (EnumValueNameMap_t::const_iterator it = ecd.valueMaps.begin(); it != ecd.valueMaps.end(); ++it)
		{
			const EnumValueItem_t& valueItem = it->second;

			const char* pItemStr = valueItem.name.c_str();

			if (behaviac::StringUtils::StringEqual(valueStr, pItemStr))
			{
				v = (T)it->first;
				return true;
			}
		}

		return false;
	}

	struct EnumClassDescriptionBSS_t
	{
		EnumClassDescription_t* descriptor;
	};

	typedef behaviac::map<behaviac::string, const EnumClassDescriptionBSS_t*>	EnumClassMap_t;
	BEHAVIAC_API EnumClassMap_t& GetEnumValueNameMaps();
	BEHAVIAC_API void CleanupEnumValueNameMaps();
}//

/**
you need to accompany DECLARE_BEHAVIAC_ENUM(ENUMCLASS_FullNameWithNamespace)
in the cpp, BEHAVIAC_BEGIN_ENUM/BEHAVIAC_END_ENUM

DECLARE_BEHAVIAC_ENUM(namespace::ENUMCLASS_FullNameWithNamespace, EnumClass) should be defined in the global namespace.
*/
#define DECLARE_BEHAVIAC_ENUM(ENUMCLASS_FullNameWithNamespace, EnumClassName)														\
    BEHAVIAC_OVERRIDE_TYPE_NAME(ENUMCLASS_FullNameWithNamespace);																	\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(ENUMCLASS_FullNameWithNamespace, BasicTypeHandlerEnum<ENUMCLASS_FullNameWithNamespace>);		\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(behaviac::vector<ENUMCLASS_FullNameWithNamespace>, BasicTypeHandlerEnum<behaviac::vector<ENUMCLASS_FullNameWithNamespace> >);\
    BEHAVIAC_API behaviac::EnumClassDescriptionBSS_t& EnumClassName##GetEnumClassValueNames();\
    BEHAVIAC_API void RegisterEnumClass(ENUMCLASS_FullNameWithNamespace*);		\
	namespace behaviac {														\
		template<>																\
		BEHAVIAC_FORCEINLINE  const behaviac::EnumClassDescription_t& GetEnumClassValueNames<ENUMCLASS_FullNameWithNamespace>(ENUMCLASS_FullNameWithNamespace*p)\
		{																		\
			RegisterEnumClass(p);												\
			behaviac::EnumClassDescriptionBSS_t& descriptorBSS = EnumClassName##GetEnumClassValueNames();\
			return *descriptorBSS.descriptor;									\
		}																		\
	}																			\
    template< typename SWAPPER >												\
    inline void SwapByteImplement(ENUMCLASS_FullNameWithNamespace& s)			\
    {																			\
        int t = (int)s;															\
        behaviacSwapByte(t);													\
        s = (ENUMCLASS_FullNameWithNamespace)t;									\
    }

#define BEHAVIAC_BEGIN_ENUM(ENUMCLASS, EnumClassName)							\
    behaviac::EnumClassDescriptionBSS_t& EnumClassName##GetEnumClassValueNames()\
    {																			\
        static behaviac::EnumClassDescriptionBSS_t s_ValueNameMap;				\
        if (!s_ValueNameMap.descriptor)											\
        {																		\
            s_ValueNameMap.descriptor = BEHAVIAC_NEW behaviac::EnumClassDescription_t;	\
        }																		\
        return s_ValueNameMap;													\
    }																			\
    void RegisterEnumClass(ENUMCLASS*)											\
    {																			\
        const char* enumClassName = behaviac::GetClassTypeName((ENUMCLASS*)0);	\
        behaviac::EnumClassMap_t& maps = behaviac::GetEnumValueNameMaps();		\
        behaviac::EnumClassMap_t::iterator it = maps.find(enumClassName);		\
        if (it != maps.end())													\
        {																		\
            return;																\
        }																		\
        behaviac::EnumClassDescriptionBSS_t& enumClassDescBSS = EnumClassName##GetEnumClassValueNames();\
        maps[enumClassName] = &enumClassDescBSS;								\
        behaviac::EnumClassDescription_t& enumClassDesc = *enumClassDescBSS.descriptor;

//deprecated, to use DECLARE_BEHAVIAC_ENUM 
#define DECLARE_BEHAVIAC_OBJECT_ENUM DECLARE_BEHAVIAC_ENUM

//deparated, to use BEHAVIAC_ENUMCLASS_DISPLAY_INFO
#define BEHAVIAC_ENUMCLASS_DISPLAYNAME(displayName_)    enumClassDesc.m_displayName = displayName_;
#define BEHAVIAC_ENUMCLASS_DESC(desc_)    enumClassDesc.m_desc = desc_;

#define BEHAVIAC_ENUMCLASS_DISPLAY_INFO(displayName_, desc_)		\
    enumClassDesc.m_displayName = displayName_;						\
    if (desc_) {enumClassDesc.m_desc = desc_;}						\
    else {enumClassDesc.m_desc = displayName_;}

namespace behaviac
{
	inline EnumValueItem_t& _defineEnumName(EnumClassDescription_t& ecd, uint32_t value, const char* nativeName, const char* name)
	{
		EnumValueItem_t& e = ecd.valueMaps[value];
		e.nativeName = nativeName;
		e.name = name;

		return e;
	}
}

#define BEHAVIAC_ENUM_ITEM(value, name)	behaviac::_defineEnumName(enumClassDesc, value, #value, name)
#define BEHAVIAC_END_ENUM()         }
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "behaviac/common/object/typehandler.h"

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// deprecated
#define DECLARE_BEHAVIAC_AGENT(agent, base) BEHAVIAC_DECLARE_AGENTTYPE(agent, base)
#define DECLARE_BEHAVIAC_OBJECT(agent, base) BEHAVIAC_DECLARE_AGENTTYPE(agent, base)

//#define BEHAVIAC_BEGIN_PROPERTIES(struct_type) BEHAVIAC_BEGIN_STRUCT(struct_type)
//#define BEHAVIAC_REGISTER_METHOD(method)
//#define BEHAVIAC_REGISTER_PROPERTY(member) BEHAVIAC_REGISTER_STRUCT_PROPERTY(member)
//#define BEHAVIAC_END_PROPERTIES() BEHAVIAC_END_STRUCT()
//
//#define BEGIN_PROPERTIES_DESCRIPTION(agent)
//#define END_PROPERTIES_DESCRIPTION()
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef _BEHAVIAC_COMMON_OBJECT_TAGOBJECT_H_
