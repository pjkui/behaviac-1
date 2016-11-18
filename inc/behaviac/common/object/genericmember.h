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

#ifndef _BEHAVIAC_COMMON_OBJECT_GENERICMEMBER_H_
#define _BEHAVIAC_COMMON_OBJECT_GENERICMEMBER_H_

#include "behaviac/common/object/member.h"
#include "behaviac/property/property.h"
#include "behaviac/property/operators.inl"
#include "behaviac/agent/agent.h"

#include "behaviac/network/network.h"

namespace behaviac {
	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE>
	class TGenericMemberBase : public IMemberBase
	{
	public:
		BEHAVIAC_DECLARE_MEMORY_OPERATORS(TGenericMemberBase);

		TGenericMemberBase(MEMBERTYPE AGENTTYPE::* memberPtr, const char* className, const char* propertyName) : IMemberBase(propertyName, className), m_memberPtr(memberPtr), m_getter(0), m_setter(0)
		{
		}

		//typedef const MEMBERTYPE& (AGENTTYPE::*Getter)();
		//typedef void(AGENTTYPE::*Setter)(const MEMBERTYPE&);

		TGenericMemberBase(const MEMBERTYPE & (AGENTTYPE::*getter)() const, void (AGENTTYPE::*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			IMemberBase(propertyName, className), m_memberPtr(0), m_getter(getter), m_setter(setter)
		{
		}

		TGenericMemberBase(const TGenericMemberBase& copy) : IMemberBase(copy), m_memberPtr(copy.m_memberPtr), m_getter(copy.m_getter), m_setter(copy.m_setter)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW TGenericMemberBase(*this);

			return p;
		}

		virtual void Load(CTagObject* parent, const IIONode* node)
		{
			HANLDERTYPE::Load(node, ((AGENTTYPE*)parent)->*m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void Save(const CTagObject* parent, IIONode* node)
		{
			HANLDERTYPE::Save(node, ((AGENTTYPE*)parent)->*m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void LoadState(CTagObject* parent, const IIONode* node)
		{
			HANLDERTYPE::LoadState(node, ((AGENTTYPE*)parent)->*m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void SaveState(const CTagObject* parent, IIONode* node)
		{
			HANLDERTYPE::SaveState(node, ((AGENTTYPE*)parent)->*m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual int READONLYFLAG() const
		{
			int flag = 0;

			if (this->m_bIsConst)
			{
				flag |= 0x1;
			}

			if (this->m_getter != 0)
			{
				flag |= 0x2;

				if (this->m_setter == 0)
				{
					flag |= 0x1;
				}
			}

			return flag;
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(value);
			BEHAVIAC_UNUSED_VAR(typeId);

			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);
			BEHAVIAC_ASSERT(false);
		}

		virtual const void* Get(const CTagObject* parent, int typeId) const
		{
			bool bOk = (typeId == GetClassTypeNumberId<System::Object>() ||
				GetClassTypeNumberId<MEMBERTYPE>() == typeId);

			BEHAVIAC_ASSERT(bOk);

			if (bOk)
			{
				if (m_memberPtr)
				{
					return &(((AGENTTYPE*)parent)->*m_memberPtr);
				}
				else if (this->m_getter)
				{
					return &(((AGENTTYPE*)parent)->*m_getter)();
				}
			}

			return 0;
		}

		virtual void SetVariable(const CTagObject* parent, const void* value, int typeId) const
		{
		}

#if BEHAVIAC_ENABLE_NETWORKD
		virtual void ReplicateProperty(Agent* pAgent)
		{
			const char* propertyName = m_propertyID.GetString();
			NetworkRole netRole = this->NETROLE();

			if (netRole != NET_ROLE_DEFAULT)
			{
				Network* pNw = Network::GetInstance();

				if (pNw && !pNw->IsSinglePlayer())
				{
					bool bSend = false;
					bool bReceive = false;

					if (pNw->IsAuthority())
					{
						if (netRole == NET_ROLE_NONAUTHORITY)
						{
							bSend = true;

						}
						else if (netRole == NET_ROLE_AUTHORITY)
						{
							bReceive = true;
						}
					}
					else
					{
						if (netRole == NET_ROLE_AUTHORITY)
						{
							bSend = true;

						}
						else if (netRole == NET_ROLE_NONAUTHORITY)
						{
							bReceive = true;
						}
					}

					if (bSend || bReceive)
					{
						int typeId = GetClassTypeNumberId<MEMBERTYPE>();

						pNw->ReplicateVariable(pAgent, propertyName, typeId, (void*) & (((AGENTTYPE*)pAgent)->*m_memberPtr), bSend);
					}
				}
			}
		}
#endif//#if BEHAVIAC_ENABLE_NETWORKD
		virtual void*       GetVariable(const CTagObject* parent, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(typeId);

			BEHAVIAC_ASSERT(!"Only works with TGenericMembers");
			return NULL;
		};

		virtual int GetTypeId() const
		{
			return GetClassTypeNumberId<MEMBERTYPE>();
		}

		virtual bool       Equal(const CTagObject* lhs, const CTagObject* rhs) const
		{
			const MEMBERTYPE& l_v = *(const MEMBERTYPE*)this->Get(lhs, GetClassTypeNumberId<MEMBERTYPE>());
			const MEMBERTYPE& r_v = *(const MEMBERTYPE*)this->Get(rhs, GetClassTypeNumberId<MEMBERTYPE>());

			return PrivateDetails::Equal(l_v, r_v);
		}

	protected:
		MEMBERTYPE AGENTTYPE::*    m_memberPtr;

		const MEMBERTYPE& (AGENTTYPE::*m_getter)() const;
		void(AGENTTYPE::*m_setter)(const MEMBERTYPE&);
	};

	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE>
	class TGenericMemberNonConst : public TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>
	{
	public:
		TGenericMemberNonConst(MEMBERTYPE AGENTTYPE::* memberPtr, const char* className, const char* propertyName) :
			TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(memberPtr, className, propertyName)
		{
		}

		TGenericMemberNonConst(const MEMBERTYPE & (AGENTTYPE::*getter)() const, void (AGENTTYPE::*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(getter, setter, className, propertyName)
		{
		}

		TGenericMemberNonConst(const TGenericMemberNonConst& copy) : TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW TGenericMemberNonConst(*this);

			return p;
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);

			if (GetClassTypeNumberId<MEMBERTYPE>() == typeId)
			{
				if (this->m_memberPtr)
				{
					((AGENTTYPE*)parent)->*this->m_memberPtr = *((MEMBERTYPE*)value);
				}
				else if (this->m_setter)
				{
					(((AGENTTYPE*)parent)->*this->m_setter)(*((MEMBERTYPE*)value));
				}
				else
				{
					BEHAVIAC_ASSERT(false, "weird, a property's setter is not set");
				}
			}
		}

	};

	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE>
	class TGenericMemberConst : public TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>
	{
	public:
		TGenericMemberConst(MEMBERTYPE AGENTTYPE::* memberPtr, const char* className, const char* propertyName) :
			TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(memberPtr, className, propertyName)
		{
			this->m_bIsConst = true;
		}

		TGenericMemberConst(const MEMBERTYPE & (AGENTTYPE::*getter)() const, void (AGENTTYPE::*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(getter, setter, className, propertyName)
		{
		}

		TGenericMemberConst(const TGenericMemberConst& copy) : TGenericMemberBase<AGENTTYPE, MEMBERTYPE, HANLDERTYPE>(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW TGenericMemberConst(*this);

			return p;
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);
			BEHAVIAC_ASSERT(false, "wierd, a const member is set value");
		}

	};


	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE, bool bIsConst>
	struct TGenericMemberSelector
	{
		typedef TGenericMemberNonConst<AGENTTYPE, MEMBERTYPE, HANLDERTYPE> TGenericMember;
	};

	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE>
	struct TGenericMemberSelector<AGENTTYPE, MEMBERTYPE, HANLDERTYPE, true>
	{
		typedef TGenericMemberConst<AGENTTYPE, MEMBERTYPE, HANLDERTYPE> TGenericMember;
	};

	template<class AGENTTYPE, class MEMBERTYPE, class HANLDERTYPE>
	class TGenericMember : public TGenericMemberSelector<AGENTTYPE, MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::TGenericMember
	{
	public:
		TGenericMember(MEMBERTYPE AGENTTYPE::* memberPtr, const char* className, const char* propertyName) :
			TGenericMemberSelector<AGENTTYPE, MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::TGenericMember(memberPtr, className, propertyName)
		{
		}

		TGenericMember(const MEMBERTYPE & (AGENTTYPE::*getter)() const, void (AGENTTYPE::*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			TGenericMemberSelector<AGENTTYPE, MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::TGenericMember(getter, setter, className, propertyName)
		{
		}

		TGenericMember(const TGenericMember& copy) : TGenericMemberSelector<AGENTTYPE, MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::TGenericMember(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW TGenericMember(*this);

			return p;
		}
	};


	template<class MEMBERTYPE, class HANLDERTYPE>
	class CStaticMemberBase : public IMemberBase
	{
	public:
		BEHAVIAC_DECLARE_MEMORY_OPERATORS(CStaticMemberBase);

		CStaticMemberBase(MEMBERTYPE* memberPtr, const char* className, const char* propertyName) :
			IMemberBase(propertyName, className), m_memberPtr(memberPtr), m_getter(0), m_setter(0)
		{
			m_bStatic = true;
		}

		CStaticMemberBase(const MEMBERTYPE & (*getter)(), void(*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			IMemberBase(propertyName, className), m_memberPtr(0), m_getter(getter), m_setter(setter)
		{
			m_bStatic = true;
		}

		CStaticMemberBase(const CStaticMemberBase& copy) :
			IMemberBase(copy), m_memberPtr(copy.m_memberPtr), m_getter(copy.m_getter), m_setter(copy.m_setter)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW CStaticMemberBase(*this);

			return p;
		}

		virtual void Load(CTagObject* parent, const IIONode* node)
		{
			BEHAVIAC_UNUSED_VAR(parent);

			HANLDERTYPE::Load(node, *m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void Save(const CTagObject* parent, IIONode* node)
		{
			BEHAVIAC_UNUSED_VAR(parent);

			HANLDERTYPE::Save(node, *m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void LoadState(CTagObject* parent, const IIONode* node)
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(node);

			HANLDERTYPE::LoadState(node, *m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual void SaveState(const CTagObject* parent, IIONode* node)
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(node);

			HANLDERTYPE::SaveState(node, *m_memberPtr, this->m_classFullName, m_propertyID);
		}

		virtual int READONLYFLAG() const
		{
			int flag = 0;

			if (this->m_bIsConst)
			{
				flag |= 0x1;
			}

			if (this->m_getter != 0)
			{
				flag |= 0x2;

				if (this->m_setter == 0)
				{
					flag |= 0x1;
				}
			}

			return flag;
		}


		virtual void GetMethodsDescription(CTagTypeDescriptor::TypesMap_t* types, const CTagObject* parent, const XmlNodeRef& xmlNode)
		{
			BEHAVIAC_UNUSED_VAR(parent);

			HANLDERTYPE::GetMethodsDescription(types, xmlNode, *m_memberPtr, this->m_classFullName, m_propertyID.GetString());
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(value);
			BEHAVIAC_UNUSED_VAR(typeId);

			BEHAVIAC_ASSERT(false);
		}

		virtual const void* Get(const CTagObject* parent, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);

			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);

			if (GetClassTypeNumberId<MEMBERTYPE>() == typeId)
			{
				if (m_memberPtr)
				{
					return (const void*)& (*m_memberPtr);
				}
				else if (this->m_getter)
				{
					return &(*m_getter)();
				}
			}

			return 0;
		}

		virtual void  SetVariable(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(typeId);

			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);

			if (GetClassTypeNumberId<MEMBERTYPE>() == typeId)
			{
				Agent* pAgent = (Agent*)parent;
				const MEMBERTYPE& v = *(MEMBERTYPE*)value;

				const char* propertyName = m_propertyID.GetString();
				//pAgent->SetVariable(propertyName, v);
				//pAgent->SetVariableRegistry(false, this, propertyName, v, this->m_classFullName, this->m_propertyID.GetID().GetUniqueID());
				BEHAVIAC_ASSERT(0);
			}
		}
		
#if BEHAVIAC_ENABLE_NETWORKD
		virtual void ReplicateProperty(Agent* pAgent)
		{
			BEHAVIAC_UNUSED_VAR(pAgent);

			const char* propertyName = m_propertyID.GetString();
			NetworkRole netRole = this->NETROLE();

			if (netRole != NET_ROLE_DEFAULT)
			{
				Network* pNw = Network::GetInstance();

				if (pNw && !pNw->IsSinglePlayer())
				{
					bool bSend = false;
					bool bReceive = false;

					if (pNw->IsAuthority())
					{
						if (netRole == NET_ROLE_NONAUTHORITY)
						{
							bSend = true;

						}
						else if (netRole == NET_ROLE_AUTHORITY)
						{
							bReceive = true;
						}
					}
					else
					{
						if (netRole == NET_ROLE_AUTHORITY)
						{
							bSend = true;

						}
						else if (netRole == NET_ROLE_NONAUTHORITY)
						{
							bReceive = true;
						}
					}

					if (bSend || bReceive)
					{
						int typeId = GetClassTypeNumberId<MEMBERTYPE>();

						pNw->ReplicateVariable(this->m_classFullName, propertyName, typeId, (void*) & (*m_memberPtr), bSend);
					}
				}
			}
		}
#endif//#if BEHAVIAC_ENABLE_NETWORKD
		virtual void*       GetVariable(const CTagObject* parent, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(typeId);

			BEHAVIAC_ASSERT(!"Only works with TGenericMembers");
			return NULL;
		}

		virtual int GetTypeId() const
		{
			return GetClassTypeNumberId<MEMBERTYPE>();
		}

		virtual bool       Equal(const CTagObject* lhs, const CTagObject* rhs) const
		{
			const MEMBERTYPE& l_v = *(const MEMBERTYPE*)this->Get(lhs, GetClassTypeNumberId<MEMBERTYPE>());
			const MEMBERTYPE& r_v = *(const MEMBERTYPE*)this->Get(rhs, GetClassTypeNumberId<MEMBERTYPE>());

			return PrivateDetails::Equal(l_v, r_v);
		}

	protected:
		MEMBERTYPE*				m_memberPtr;
		const MEMBERTYPE& (*m_getter)();
		void(*m_setter)(const MEMBERTYPE&);
	};

	template<class MEMBERTYPE, class HANLDERTYPE>
	class CStaticMemberNonConst : public CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>
	{
	public:
		CStaticMemberNonConst(MEMBERTYPE* memberPtr, const char* className, const char* propertyName) :
			CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(memberPtr, className, propertyName)
		{
		}

		CStaticMemberNonConst(const MEMBERTYPE & (*getter)(), void(*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(getter, setter, className, propertyName)
		{
		}

		CStaticMemberNonConst(const CStaticMemberNonConst& copy) : CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW CStaticMemberNonConst(*this);

			return p;
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_UNUSED_VAR(parent);
			BEHAVIAC_UNUSED_VAR(typeId);
			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);

			if (GetClassTypeNumberId<MEMBERTYPE>() == typeId)
			{
				if (this->m_memberPtr)
				{
					*this->m_memberPtr = *((MEMBERTYPE*)value);
				}
				else if (this->m_setter)
				{
					(*this->m_setter)(*((MEMBERTYPE*)value));
				}
			}
		}

	};

	template<class MEMBERTYPE, class HANLDERTYPE>
	class CStaticMemberConst : public CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>
	{
	public:
		CStaticMemberConst(MEMBERTYPE* memberPtr, const char* className, const char* propertyName) :
			CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(memberPtr, className, propertyName)
		{
		}

		CStaticMemberConst(const MEMBERTYPE & (*getter)(), void(*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(getter, setter, className, propertyName)
		{
		}

		CStaticMemberConst(const CStaticMemberConst& copy) : CStaticMemberBase<MEMBERTYPE, HANLDERTYPE>(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW CStaticMemberConst(*this);

			return p;
		}

		virtual void Set(const CTagObject* parent, const void* value, int typeId) const
		{
			BEHAVIAC_ASSERT(GetClassTypeNumberId<MEMBERTYPE>() == typeId);
			BEHAVIAC_ASSERT(false, "wierd, a const member is set value");
		}

	};


	template<class MEMBERTYPE, class HANLDERTYPE, bool bIsConst>
	struct CStaticMemberSelector
	{
		typedef CStaticMemberNonConst<MEMBERTYPE, HANLDERTYPE> CStaticMember;
	};

	template<class MEMBERTYPE, class HANLDERTYPE>
	struct CStaticMemberSelector<MEMBERTYPE, HANLDERTYPE, true>
	{
		typedef CStaticMemberConst<MEMBERTYPE, HANLDERTYPE> CStaticMember;
	};

	template<class MEMBERTYPE, class HANLDERTYPE>
	class CStaticMember : public CStaticMemberSelector<MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::CStaticMember
	{
	public:
		CStaticMember(MEMBERTYPE* memberPtr, const char* className, const char* propertyName) :
			CStaticMemberSelector<MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::CStaticMember(memberPtr, className, propertyName)
		{
		}

		CStaticMember(const MEMBERTYPE & (*getter)(), void(*setter)(const MEMBERTYPE&), const char* className, const char* propertyName) :
			CStaticMemberSelector<MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::CStaticMember(getter, setter, className, propertyName)
		{
		}

		CStaticMember(const CStaticMember& copy) : CStaticMemberSelector<MEMBERTYPE, HANLDERTYPE, Meta::IsConst<MEMBERTYPE>::Result>::CStaticMember(copy)
		{}

		virtual IMemberBase* clone() const
		{
			IMemberBase* p = BEHAVIAC_NEW CStaticMember(*this);

			return p;
		}
	};
}//


namespace behaviac
{
	BEHAVIAC_FORCEINLINE IMemberBase& _addMember(CTagObject::MembersContainer& members, IMemberBase* _member)
	{
		members.membersVector.push_back(_member);
		members.membersMap[_member->GetID().GetID()] = _member;
		return *_member;
	}

	namespace CMemberFactory
	{
		template<template <class T> class HANLDERTYPE, class AGENTTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, MEMBERTYPE AGENTTYPE::* memberPtr)
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef TGenericMember<AGENTTYPE, MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(memberPtr, className, propertyName);
		}

		template<template <class T> class HANLDERTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, MEMBERTYPE* memberPtr)
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef CStaticMember<MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(memberPtr, className, propertyName);
		}

		template<template <class T> class HANLDERTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, const MEMBERTYPE & (*getter)())
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef CStaticMember<MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(getter, 0, className, propertyName);
		}

		template<template <class T> class HANLDERTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, const MEMBERTYPE & (*getter)(), void(*setter)(const MEMBERTYPE&))
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef CStaticMember<MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(getter, setter, className, propertyName);
		}

		template<class HANLDERTYPE, class AGENTTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, MEMBERTYPE AGENTTYPE::* memberPtr)
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef TGenericMember<AGENTTYPE, MEMBERTYPE, HANLDERTYPE> MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(memberPtr, className, propertyName);
		}

		template<template <class T> class HANLDERTYPE, class AGENTTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, const MEMBERTYPE & (AGENTTYPE::*getter)() const)
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef TGenericMember<AGENTTYPE, MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(getter, 0, className, propertyName);
		}

		template<template <class T> class HANLDERTYPE, class AGENTTYPE, class MEMBERTYPE>
		static IMemberBase* Bind(const char* className, const char* propertyName, const MEMBERTYPE & (AGENTTYPE::*getter)() const, void (AGENTTYPE::*setter)(const MEMBERTYPE&))
		{
			BEHAVIAC_UNUSED_VAR(className);
			typedef TGenericMember<AGENTTYPE, MEMBERTYPE, HANLDERTYPE<MEMBERTYPE> > MEMBERTYPEType;
			return BEHAVIAC_NEW MEMBERTYPEType(getter, setter, className, propertyName);
		}
	};
}//

#endif // #ifndef _BEHAVIAC_COMMON_OBJECT_GENERICMEMBER_H_
