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

#ifndef _BEHAVIAC_COMMON_OBJECT_TYPEHANDLER_H_
#define _BEHAVIAC_COMMON_OBJECT_TYPEHANDLER_H_

#include "behaviac/common/serialization/ionode.h"

#include "behaviac/common/rttibase.h"
#include "behaviac/common/object/tagobject.h"

#include "behaviac/property/vector_ext.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DefaultTypeHandler behaviac::GenericTypeHandler

namespace behaviac
{
	template<class MEMBERTYPE>
	struct GenericTypeHandler
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			// If property ID is empty use the current node to load
			const IIONode* memberNode = (propertyID.GetID().IsValid()) ? node->findNodeChild(propertyID) : node;

			if (memberNode)
			{
				member.Load(memberNode);
				return true;
			}

			return false;
		}

		static void Save(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			// Create a new node if the property ID is not empty, otherwise serialize it to the current node
			IIONode* memberNode = (propertyID.GetID().IsValid()) ? node->newNodeChild(propertyID) : node;
			member.Save(memberNode);
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			// If property ID is empty use the current node to load
			const IIONode* memberNode = (propertyID.GetID().IsValid()) ? node->findNodeChild(propertyID) : node;

			if (memberNode)
			{
				member.LoadState(memberNode);
				return true;
			}

			return false;
		}

		static void SaveState(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			// Create a new node if the property ID is not empty, otherwise serialize it to the current node
			IIONode* memberNode = (propertyID.GetID().IsValid()) ? node->newNodeChild(propertyID) : node;
			member.SaveState(memberNode);
		}

		static void LoadFromXML(const IIONode& xmlNode, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(xmlNode);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(propertyName);
			CIOID serializationId(propertyName);
			GenericTypeHandler<MEMBERTYPE>::Load(&xmlNode, member, szClassTypeName, serializationId);
		}

		static void SaveToXML(IIONode& xmlNode, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(xmlNode);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyName);
			BEHAVIAC_ASSERT(0, "unimplmented");
		}
	};

	template<class MEMBERTYPE>
	struct NoDescriptionTypeHandler
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			const IIONode* memberNode = node->findNodeChild(propertyID);

			if (memberNode)
			{
				member.Load(memberNode);
				return true;
			}

			return false;
		}

		static void Save(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			IIONode* memberNode = node->newNodeChild(propertyID);
			member.Save(memberNode);
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			const IIONode* memberNode = node->findNodeChild(propertyID);

			if (memberNode)
			{
				member.LoadState(memberNode);
				return true;
			}

			return false;
		}

		static void SaveState(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(member);

			IIONode* memberNode = node->newNodeChild(propertyID);
			member.SaveState(memberNode);
		}

	};

	template<class MEMBERTYPE>
	struct GenericTypeHandler<MEMBERTYPE*>
	{
		static void Load(const IIONode* node, MEMBERTYPE*& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				const IIONode* memberNode = node->findNodeChild(propertyID);

				if (memberNode)
				{
					member->Load(memberNode);
				}
				else
				{
					const char* valueStr = node->getAttrRaw(propertyID);

					if (valueStr && string_icmp(valueStr, "null") == 0)
					{
						member = 0;
					}
				}
			}
		}

		template < typename T, bool bAgent >
		struct SaveSelector
		{
			static void Save(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
			{
				BEHAVIAC_UNUSED_VAR(node);
				BEHAVIAC_UNUSED_VAR(member);
				BEHAVIAC_UNUSED_VAR(szClassTypeName);
				BEHAVIAC_UNUSED_VAR(propertyID);

				if (member)
				{
					IIONode* memberNode = node->newNodeChild(propertyID);
					member->Save(memberNode);
				}
			}
		};

		template < typename T >
		struct SaveSelector < T, true >
		{
			static void Save(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
			{
				BEHAVIAC_UNUSED_VAR(node);
				BEHAVIAC_UNUSED_VAR(member);
				BEHAVIAC_UNUSED_VAR(szClassTypeName);
				BEHAVIAC_UNUSED_VAR(propertyID);

				//if (member)
				//{
				//	IIONode* memberNode = node->newNodeChild(propertyID);
				//	member->Save(memberNode);
				//}
			}
		};

		static void Save(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			SaveSelector<MEMBERTYPE, Meta::IsRefType<MEMBERTYPE>::Result>::Save(node, member, szClassTypeName, propertyID);
		}

		static void LoadState(const IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				const IIONode* memberNode = node->findNodeChild(propertyID);

				if (memberNode)
				{
					member->LoadState(memberNode);
				}
			}
		}

		static void SaveState(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				IIONode* memberNode = node->newNodeChild(propertyID);
				member->SaveState(memberNode);
			}
		}

		static void SaveToXML(IIONode& xmlNode, const MEMBERTYPE* member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(xmlNode);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyName);
			BEHAVIAC_ASSERT(0, "unimplmented");
		}
	};

	template<class MEMBERTYPE, uint32_t ArraySize >
	struct GenericTypeHandler< MEMBERTYPE[ArraySize] >
	{
		typedef MEMBERTYPE Array[ArraySize];

		static void Load(const IIONode* node, Array& array, const CIOID& propertyID)
		{
			const IIONode* arrayNode = node->findNodeChild(propertyID);

			if (arrayNode)
			{
				for (uint32_t i = 0; i < ArraySize; ++i)
				{
					char name[16];
					string_sprintf(name, "Element%d", i);
					GenericTypeHandler< MEMBERTYPE >::Load(arrayNode, array[i], name);
				}
			}
		}

		static void Save(IIONode* node, Array& array, const CIOID& propertyID)
		{
			IIONode* arrayNode = node->newNodeChild(propertyID);

			for (uint32_t i = 0; i < ArraySize; ++i)
			{
				char name[16];
				string_sprintf(name, "Element%d", i);
				GenericTypeHandler< MEMBERTYPE >::Save(arrayNode, array[i], name);
			}
		}

		static void LoadState(const IIONode* node, Array& array, const CIOID& propertyID)
		{
			const IIONode* arrayNode = node->findNodeChild(propertyID);

			if (arrayNode)
			{
				for (uint32_t i = 0; i < ArraySize; ++i)
				{
					char name[16];
					string_sprintf(name, "Element%d", i);
					GenericTypeHandler< MEMBERTYPE >::LoadState(arrayNode, array[i], name);
				}
			}
		}

		static void SaveState(IIONode* node, const behaviac::vector<MEMBERTYPE>& array, const CIOID& propertyID)
		{
			IIONode* arrayNode = node->newNodeChild(propertyID);

			for (uint32_t i = 0; i < ArraySize; ++i)
			{
				char name[16];
				string_sprintf(name, "Element%d", i);
				GenericTypeHandler< MEMBERTYPE >::SaveState(arrayNode, array[i], name);
			}
		}

	};

	template<class MEMBERTYPE>
	struct GenericTypeHandler<behaviac::vector<MEMBERTYPE> >
	{
		static void Load(const IIONode* node, behaviac::vector<MEMBERTYPE>& _array, const char* classTypeName, const CIOID& propertyID)
		{
			const char* attrVauleStr = node->getAttrRaw(propertyID);

			if (attrVauleStr)
			{
				StringUtils::ParseString(attrVauleStr, _array);
			}
		}

		static void Save(IIONode* node, behaviac::vector<MEMBERTYPE>& _array, const char* classTypeName, const CIOID& propertyID)
		{
			IIONode* arrayNode = node->newNodeChild(propertyID);

			uint32_t ArraySize = _array.size();

			for (uint32_t i = 0; i < ArraySize; ++i)
			{
				char name[16];
				string_sprintf(name, "Element%d", i);
				behaviac::CIOID id(name);
				GenericTypeHandler< MEMBERTYPE >::Save(arrayNode, _array[i], classTypeName, id);
			}
		}

		static void LoadState(const IIONode* node, behaviac::vector<MEMBERTYPE>& _array, const char* classTypeName, const CIOID& propertyID)
		{
			const IIONode* arrayNode = node->findNodeChild(propertyID);

			if (arrayNode)
			{
				uint32_t ArraySize = _array.size();

				for (uint32_t i = 0; i < ArraySize; ++i)
				{
					char name[16];
					string_sprintf(name, "Element%d", i);
					behaviac::CIOID id(name);
					GenericTypeHandler< MEMBERTYPE >::LoadState(arrayNode, _array[i], classTypeName, id);
				}
			}
		}

		static void SaveState(IIONode* node, const behaviac::vector<MEMBERTYPE>& _array, const char* classTypeName, const CIOID& propertyID)
		{
			IIONode* arrayNode = node->newNodeChild(propertyID);
			uint32_t ArraySize = _array.size();

			for (uint32_t i = 0; i < ArraySize; ++i)
			{
				char name[16];
				string_sprintf(name, "Element%d", i);
				behaviac::CIOID id(name);
				GenericTypeHandler< MEMBERTYPE >::SaveState(arrayNode, _array[i], classTypeName, id);
			}
		}

	};


	template<class MEMBERTYPE>
	struct NoChildTypeHandler
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			member.Load(node);
			return true;
		}

		static void Save(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			member.Save(node);
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			member.LoadState(node);
			return true;
		}

		static void SaveState(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			member.SaveState(node);
		}
		
	};

	template<class MEMBERTYPE>
	struct NoChildTypeHandler<MEMBERTYPE*>
	{
		static void Load(const IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				member->Load(node);
			}
		}

		static void Save(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				member->Save(node);
			}
		}

		static void LoadState(const IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				member->LoadState(node);
			}
		}

		static void SaveState(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			if (member)
			{
				member->SaveState(node);
			}
		}

	};

	template<class MEMBERTYPE>
	struct BasicTypeHandlerBase
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			return false;
		}

		static void Save(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);

			//if (szClassTypeName)
			//{
			//	string propertyName = FormatString("%s::%s", szClassTypeName, propertyID.GetString());
			//	CIOID pId(propertyName.c_str());

			//	node->setAttr(pId, member);
			//}
			//else
			{
				node->setAttr(propertyID, member);
			}
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			return false;
		}

		static void SaveState(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			//if (szClassTypeName)
			//{
			//	string propertyName = FormatString("%s::%s", szClassTypeName, propertyID.GetString());
			//	CIOID pId(propertyName.c_str());

			//	node->setAttr(pId, member);
			//}
			//else
			{
				node->setAttr(propertyID, member);
			}
		}

		static void LoadFromXML(const IIONode& node, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(propertyName);
			CIOID serializationId(propertyName);
			GenericTypeHandler<MEMBERTYPE>::Load(&node, member, szClassTypeName, serializationId);
		}

		static void SaveToXML(IIONode& node, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(propertyName);
			CIOID serializationId(propertyName);
			GenericTypeHandler<MEMBERTYPE>::Save(&node, member, szClassTypeName, serializationId);
		}
	};

	template<class MEMBERTYPE>
	struct BasicTypeHandlerNonConst : public BasicTypeHandlerBase<MEMBERTYPE>
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			//if (szClassTypeName)
			//{
			//	string propertyName = FormatString("%s::%s", szClassTypeName, propertyID.GetString());
			//	CIOID pId(propertyName.c_str());

			//	return node->getAttr(pId, member);
			//}
			//else
			{
				return node->getAttr(propertyID, member);
			}
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);

			//if (szClassTypeName)
			//{
			//	string propertyName = FormatString("%s::%s", szClassTypeName, propertyID.GetString());
			//	CIOID pId(propertyName.c_str());

			//	return node->getAttr(pId, member);
			//}
			//else
			{
				return node->getAttr(propertyID, member);
			}
		}


	};

	template<class MEMBERTYPE>
	struct BasicTypeHandlerConst : public BasicTypeHandlerBase<MEMBERTYPE>
	{
	};

	template<class MEMBERTYPE, bool bIsConst>
	struct BasicTypeHandlerSelector
	{
		typedef BasicTypeHandlerNonConst<MEMBERTYPE> BasicTypeHandler;
	};

	template<class MEMBERTYPE>
	struct BasicTypeHandlerSelector<MEMBERTYPE, true>
	{
		typedef BasicTypeHandlerConst<MEMBERTYPE> BasicTypeHandler;
	};

	template<class MEMBERTYPE>
	struct BasicTypeHandler : public BasicTypeHandlerSelector<MEMBERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::BasicTypeHandler
	{
	};

	template<class MEMBERTYPE>
	struct BasicTypeHandlerEnum
	{
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);

			return node->getAttr(propertyID, member);
		}

		static void Save(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);

			node->setAttr(propertyID, member);
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			return node->getAttr(propertyID, member);
		}

		static void SaveState(IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			node->setAttr(propertyID, member);
		}

		static void LoadFromXML(const IIONode& node, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(propertyName);
			CIOID serializationId(propertyName);
			GenericTypeHandler<MEMBERTYPE>::Load(&node, member, szClassTypeName, serializationId);
		}

		static void SaveToXML(IIONode& node, MEMBERTYPE& member, const char* szClassTypeName, const char* propertyName)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(propertyName);
			CIOID serializationId(propertyName);
			GenericTypeHandler<MEMBERTYPE>::Save(&node, member, szClassTypeName, serializationId);
		}
	};
}//

#define BEHAVIAC_SPECIALIZE_TYPE_HANDLER(typeToSpecialize, specializedHandler)						\
	namespace behaviac {																			\
		template<>  struct GenericTypeHandler< typeToSpecialize > : public specializedHandler{};	\
		template<>  struct NoChildTypeHandler< typeToSpecialize > : public specializedHandler{};	\
	}

	// Specialize types supported by the ixml interface
	BEHAVIAC_SPECIALIZE_TYPE_HANDLER(const char*, BasicTypeHandler<const char*>);
	BEHAVIAC_SPECIALIZE_TYPE_HANDLER(char*, BasicTypeHandler<char*>);

#undef BEHAVIAC_DECLARE_PRIMITE_TYPE
#define BEHAVIAC_DECLARE_PRIMITE_TYPE(type, typeNameStr)												\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(type, BasicTypeHandler<type>);										\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(const type, BasicTypeHandler<const type>);							\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(const type&, BasicTypeHandler<const type>);						\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(behaviac::vector<type>, BasicTypeHandler<behaviac::vector<type> >)	\
    BEHAVIAC_SPECIALIZE_TYPE_HANDLER(std::vector<type>, BasicTypeHandler<std::vector<type> >)

	BEHAVIAC_M_PRIMITIVE_TYPES();

#undef BEHAVIAC_DECLARE_SPECIALIZE_TYPE_HANDLER
#define BEHAVIAC_DECLARE_SPECIALIZE_TYPE_HANDLER(type)		\
    BEHAVIAC_DECLARE_PRIMITE_TYPE(type, #type)				\
    BEHAVIAC_OVERRIDE_TYPE_NAME(type);

#define BEHAVIAC_M_SPECIALIZE_TYPE_HANDLER_COMPOUND()						\
    BEHAVIAC_DECLARE_SPECIALIZE_TYPE_HANDLER(CStringCRC);					

	BEHAVIAC_M_SPECIALIZE_TYPE_HANDLER_COMPOUND()

		// out side of namespace
#define _BEHAVIAC_SPECIALIZE_TYPE_VECTOR_HANDLER_(fullTypeNameWithNamespace)							\
		BEHAVIAC_SPECIALIZE_TYPE_HANDLER(behaviac::vector<fullTypeNameWithNamespace>, BasicTypeHandler<behaviac::vector<fullTypeNameWithNamespace> >);

		// out side of namespace
#define BEHAVIAC_DECLARE_TYPE_VECTOR_HANDLER(fullTypeNameWithNamespace)					\
	BEHAVIAC_OVERRIDE_TYPE_NAME(behaviac::vector<fullTypeNameWithNamespace>);			\
	_BEHAVIAC_SPECIALIZE_TYPE_VECTOR_HANDLER_(fullTypeNameWithNamespace);

namespace behaviac {
		// Inherit from this to implement only necessary functions
		template <class MEMBERTYPE>
	class CNotImplementedTypeHandler
	{
	public:
		static bool Load(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			BEHAVIAC_LOGERROR("Not implemented");
			return false;
		}

		static void Save(IIONode* node, MEMBERTYPE const& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			BEHAVIAC_LOGERROR("Not implemented");
		}

		static bool LoadState(const IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			BEHAVIAC_LOGERROR("Not implemented");
			return false;
		}

		static void SaveState(IIONode* node, MEMBERTYPE const& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			BEHAVIAC_UNUSED_VAR(node);
			BEHAVIAC_UNUSED_VAR(member);
			BEHAVIAC_UNUSED_VAR(szClassTypeName);
			BEHAVIAC_UNUSED_VAR(propertyID);

			BEHAVIAC_LOGERROR("Not implemented");
		}

	};

	// This is a utility class to avoid explicit specification of the type the
	// handler must take, it will instead be deduced from the function argument.
	template <template <class T> class MemberHandler>
	class CHandlerGuesser
	{
	public:
		template <class offsetmemberType>
		static void Load(const IIONode* node, offsetmemberType& offsetmember, const char* szClassTypeName, const CIOID& propertyID)
		{
			MemberHandler<offsetmemberType>::Load(node, offsetmember, szClassTypeName, propertyID);
		}

		template <class offsetmemberType>
		static void Save(IIONode* node, const offsetmemberType& offsetmember, const char* szClassTypeName, const CIOID& propertyID)
		{
			MemberHandler<offsetmemberType>::Save(node, offsetmember, szClassTypeName, propertyID);
		}

		template <class offsetmemberType>
		static void LoadState(const IIONode* node, offsetmemberType& offsetmember, const char* szClassTypeName, const CIOID& propertyID)
		{
			MemberHandler<offsetmemberType>::LoadState(node, offsetmember, szClassTypeName, propertyID);
		}

		template <class offsetmemberType>
		static void SaveState(IIONode* node, const offsetmemberType& offsetmember, const char* szClassTypeName, const CIOID& propertyID)
		{
			MemberHandler<offsetmemberType>::SaveState(node, offsetmember, szClassTypeName, propertyID);
		}
	};

	// Non Virtual Pointer Type Handler
	template<class MEMBERTYPE>
	struct NoVirtualPointerTypeHandler : public CNotImplementedTypeHandler<MEMBERTYPE>
	{
	};

	template<class MEMBERTYPE>
	struct NoVirtualPointerTypeHandler<MEMBERTYPE*>
	{
		static void Load(const IIONode* node, MEMBERTYPE*& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			member = BEHAVIAC_NEW MEMBERTYPE;
			const IIONode* memberNode = propertyID.GetID().IsValid() ? node->findNodeChild(propertyID) : node;

			if (memberNode)
			{
				member->Load(memberNode);
			}
		}

		static void Save(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			if (member)
			{
				IIONode* memberNode = propertyID.GetID().IsValid() ? node->newNodeChild(propertyID) : node;
				member->Save(memberNode);
			}
		}

		static void LoadState(const IIONode* node, MEMBERTYPE*& member, const char* szClassTypeName, const CIOID& propertyID)
		{
			// Currently not supported because the behavior could be error prone, because
			// of the possibility that the user would pass a vector that is not empty.
			CNotImplementedTypeHandler<MEMBERTYPE*>::LoadState(node, member, szClassTypeName, propertyID);
		}

		static void SaveState(IIONode* node, MEMBERTYPE* member, const char* szClassTypeName, const CIOID& propertyID)
		{
			// Currently not implemented because it is assumed that LoadState() is error prone in this case.
			CNotImplementedTypeHandler<MEMBERTYPE*>::SaveState(node, member, szClassTypeName, propertyID);
		}

	};
}//

#include "behaviac/common/object/typehandlerex.h"

#endif // #ifndef _BEHAVIAC_COMMON_OBJECT_TYPEHANDLER_H_
