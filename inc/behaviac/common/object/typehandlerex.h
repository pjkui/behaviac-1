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

#ifndef _BEHAVIAC_COMMON_OBJECT_TYPEHANDLEREX_H_
#define _BEHAVIAC_COMMON_OBJECT_TYPEHANDLEREX_H_

namespace behaviac {
	// please use the derivation after this class to not have number of parameters errors
	template<class MEMBERTYPE, bool bClearOnLoad>
	struct BasicVectorHandlerWithClearOption
	{
		template <bool State>
		static bool LoadHelper(const behaviac::IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			const behaviac::IIONode* childNode = node->findNodeChild(propertyID);

			if (childNode)
			{
				uint32_t childCount = childNode->getChildCount();

				if (bClearOnLoad)
				{
					member.resize(childCount);
				}

				for (uint32_t i = 0; i < childCount; i++)
				{
					const behaviac::IIONode* elementNode = childNode->getChild(i);

					if (!State)
					{
						CHandlerGuesser<GenericTypeHandler>::Load(elementNode, member[i], behaviac::CIOID(0xDCB67730, "Value"));

					}
					else
					{
						CHandlerGuesser<GenericTypeHandler>::LoadState(elementNode, member[i], behaviac::CIOID(0xDCB67730, "Value"));
					}
				}

				return true;
			}

			return false;
		}

		template <bool State>
		static void SaveHelper(behaviac::IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			behaviac::IIONode* childNode = node->newNodeChild(propertyID);

			for (uint32_t i = 0; i < member.size(); i++)
			{
				behaviac::IIONode* elementNode = childNode->newNodeChild(behaviac::CIOID(0xE7CAC750, "Elem"));

				if (!State)
				{
					CHandlerGuesser<GenericTypeHandler>::Save(elementNode, member[i], behaviac::CIOID(0xDCB67730, "Value"));

				}
				else
				{
					CHandlerGuesser<GenericTypeHandler>::SaveState(elementNode, member[i], behaviac::CIOID(0xDCB67730, "Value"));
				}
			}
		}

		static bool Load(const behaviac::IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			return LoadHelper< false >(node, member, szClassTypeName, propertyID);
		}

		static void Save(behaviac::IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			SaveHelper< false >(node, member, szClassTypeName, propertyID);
		}

		static bool LoadState(const behaviac::IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			return LoadHelper< true >(node, member, szClassTypeName, propertyID);
		}

		static void SaveState(behaviac::IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			SaveHelper< true >(node, member, szClassTypeName, propertyID);
		}

	};

	template<class MEMBERTYPE>
	struct BasicVectorHandler : public BasicVectorHandlerWithClearOption<MEMBERTYPE, true>
	{
	};

	template<class MEMBERTYPE>
	struct BasicVectorHandlerNoClear : public BasicVectorHandlerWithClearOption<MEMBERTYPE, false>
	{
	};

	template<class MEMBERTYPE>
	struct BasicMapHandler
	{
		static bool Load(const behaviac::IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			const behaviac::IIONode* childNode = node->findNodeChild(propertyID);

			if (childNode)
			{
				uint32_t childCount = childNode->getChildCount();

				for (uint32_t i = 0; i < childCount; i++)
				{
					const behaviac::IIONode* elementNode = childNode->getChild(i);
					typename MEMBERTYPE::key_type key;
					typename MEMBERTYPE::MappedType value;
					elementNode->getAttr(behaviac::CIOID(0xB2DDED49, "Key"), key);
					elementNode->getAttr(behaviac::CIOID(0xDCB67730, "Value"), value);
					member.insert(typename MEMBERTYPE::value_type(key, value));
				}

				return true;
			}

			return false;
		}

		static void Save(behaviac::IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			behaviac::IIONode* childNode = node->newNodeChild(propertyID);
			typename MEMBERTYPE::const_iterator it = member.begin();
			typename MEMBERTYPE::const_iterator itEnd = member.end();

			for (; it != itEnd; ++it)
			{
				behaviac::IIONode* elementNode = childNode->newNodeChild(behaviac::CIOID(0xE7CAC750, "Elem"));
				elementNode->setAttr(behaviac::CIOID(0xB2DDED49, "Key"), (*it).first);
				elementNode->setAttr(behaviac::CIOID(0xDCB67730, "Value"), (*it).second);
			}
		}

		static bool LoadState(const behaviac::IIONode* node, MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			return Load(node, member, szClassTypeName, propertyID);
		}

		static void SaveState(behaviac::IIONode* node, const MEMBERTYPE& member, const char* szClassTypeName, const behaviac::CIOID& propertyID)
		{
			Save(node, member, szClassTypeName, propertyID);
		}

	};
}//

#endif // #ifndef _BEHAVIAC_COMMON_OBJECT_TYPEHANDLEREX_H_
