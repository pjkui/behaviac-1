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

#include "behaviac/common/object/tagobject.h"
#include "behaviac/common/object/member.h"
#include "behaviac/common/logger/logmanager.h"
#include "behaviac/common/member.h"

namespace behaviac
{
	CTagObjectDescriptorBSS behaviac::CTagObject::ms_descriptor;
	CTagObjectDescriptor& behaviac::CTagObject::GetObjectDescriptorDirectly()
	{
		return ms_descriptor.Get();
	}

	static TagObjectDescriptorMap_t* s_objectDescriptorMap;

	TagObjectDescriptorMap_t& GetTagObjectDescriptorMaps()
	{
		if (!s_objectDescriptorMap)
		{
			s_objectDescriptorMap = BEHAVIAC_NEW TagObjectDescriptorMap_t;
		}

		BEHAVIAC_ASSERT(s_objectDescriptorMap);
		return *s_objectDescriptorMap;
	}

	void CleanupTagObjectDescriptorMaps()
	{
		if (s_objectDescriptorMap)
		{
			TagObjectDescriptorMap_t* classMaps = s_objectDescriptorMap;

			for (TagObjectDescriptorMap_t::iterator it = classMaps->begin(); it != classMaps->end(); ++it)
			{
				//const behaviac::string& className = it->first;
				CTagObjectDescriptorBSS* pObejctDesc = (CTagObjectDescriptorBSS*)it->second;
				pObejctDesc->Cleanup();
			}

			s_objectDescriptorMap->clear();
			BEHAVIAC_DELETE(s_objectDescriptorMap);
			s_objectDescriptorMap = 0;
		}

		behaviac::CTagObject::CleanupObjectDescriptor();
	}

	void CTagObjectDescriptorBSS::Cleanup()
	{
		CTagObjectDescriptor* pObejctDescP = this->m_descriptor;

		if (pObejctDescP)
		{
			for (CTagObjectDescriptor::MembersVector_t::iterator it = pObejctDescP->ms_members.membersVector.begin();
				it != pObejctDescP->ms_members.membersVector.end(); ++it)
			{
				behaviac::IMemberBase* m = *it;

				BEHAVIAC_DELETE(m);
			}

			pObejctDescP->ms_members.membersMap.clear();
			pObejctDescP->ms_members.membersVector.clear();

			if (pObejctDescP->ms_isInitialized)
			{
				BEHAVIAC_ASSERT(pObejctDescP->ms_isInitialized);
				pObejctDescP->ms_isInitialized = false;
			}

			BEHAVIAC_DELETE(this->m_descriptor);
			this->m_descriptor = 0;
		}
	}

	const behaviac::IMemberBase* CTagObjectDescriptor::GetMember(const behaviac::CStringCRC& propertyId) const
	{
		CTagObjectDescriptor::MembersMap_t::const_iterator it = this->ms_members.membersMap.find(propertyId);

		if (it != this->ms_members.membersMap.end())
		{
			const behaviac::IMemberBase* pMember = it->second;

			BEHAVIAC_ASSERT(pMember->GetID().GetID() == propertyId);

			return pMember;
		}//for

		if (this->m_parent)
		{
			const behaviac::IMemberBase* m = this->m_parent->GetMember(propertyId);

			return m;
		}

		return 0;
	}

#if BEHAVIAC_ENABLE_NETWORKD
	void CTagObjectDescriptor::ReplicateProperties(behaviac::CTagObject* parent)
	{
		if (this->m_parent)
		{
			this->m_parent->ReplicateProperties();
		}

		CTagObjectDescriptor::MembersVector_t::const_iterator it = this->ms_members.membersVector.begin();
		CTagObjectDescriptor::MembersVector_t::const_iterator itEnd = this->ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;

			m->ReplicateProperty(parent);
		}
	}
#endif

	void CTagObjectDescriptor::Load(behaviac::CTagObject* parent, const behaviac::IIONode* node) const
	{
		MembersVector_t::const_iterator it = ms_members.membersVector.begin();
		MembersVector_t::const_iterator itEnd = ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;
			m->Load(parent, node);
		}

		if (this->m_parent)
		{
			this->m_parent->Load(parent, node);
		}
	}

	void CTagObjectDescriptor::Save(const behaviac::CTagObject* parent, behaviac::IIONode* node) const
	{
		if (this->m_parent)
		{
			this->m_parent->Save(parent, node);
		}

		MembersVector_t::const_iterator it = ms_members.membersVector.begin();
		MembersVector_t::const_iterator itEnd = ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;
			m->Save(parent, node);
		}
	}

	void CTagObjectDescriptor::LoadState(behaviac::CTagObject* parent, const behaviac::IIONode* node) const
	{
		MembersVector_t::const_iterator it = ms_members.membersVector.begin();
		MembersVector_t::const_iterator itEnd = ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;
			m->LoadState(parent, node);
		}

		if (this->m_parent)
		{
			this->m_parent->LoadState(parent, node);
		}
	}

	void CTagObjectDescriptor::SaveState(const behaviac::CTagObject* parent, behaviac::IIONode* node) const
	{
		if (this->m_parent)
		{
			this->m_parent->SaveState(parent, node);
		}

		MembersVector_t::const_iterator it = ms_members.membersVector.begin();
		MembersVector_t::const_iterator itEnd = ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;
			m->SaveState(parent, node);
		}
	}


	void CTagObjectDescriptor::PushBackMember(MembersContainer& inMembers, behaviac::IMemberBase* toAddMember)
	{
		//behaviac::IMemberBase* p2 = toAddMember->clone();
		//inMembers[toAddMember->GetID().GetString()] = p2;
		inMembers.membersVector.push_back(toAddMember);
		inMembers.membersMap[toAddMember->GetID().GetID()] = toAddMember;
	}

	CTagObjectDescriptor& behaviac::CTagObject::GetObjectDescriptor()
	{
		return ms_descriptor.Get();
	}

	void behaviac::CTagObject::CleanupObjectDescriptor()
	{
		ms_descriptor.Cleanup();
	}

	void behaviac::CTagObject::LoadFromXML(const behaviac::XmlConstNodeRef& xmlNode)
	{
		behaviac::CTextNode textNode(xmlNode);
		Load(&textNode);
	}

	void behaviac::CTagObject::SaveToXML(const behaviac::XmlNodeRef& xmlNode)
	{
		behaviac::CTextNode textNode(xmlNode);
		Save(&textNode);
	}

	void behaviac::CTagObject::Load(const behaviac::IIONode* node)
	{
		GetDescriptor().Load(this, node);
	}

	void behaviac::CTagObject::Save(behaviac::IIONode* node) const
	{
		GetDescriptor().Save(this, node);
	}

#ifdef USE_METHOD_PARAMS_SYSTEM
	void behaviac::CTagObject::vCallOld(const behaviac::CStringCRC& name, const CTagMethodParams& params)
	{
		const CTagObjectDescriptor& descriptor = GetDescriptor();

		for (size_t i = 0; i < descriptor.ms_methods.size(); i++)
		{
			behaviac::CMethodBase* method = descriptor.ms_methods[i];

			if (method->GetID() == name)
			{
				method->vCallOld(this, params);
				return;
			}
		}
	}

	void behaviac::CTagObject::GetMethodParams(const behaviac::CStringCRC& name, CTagMethodParams& out_param)
	{
		const CTagObjectDescriptor& descriptor = GetDescriptor();

		for (size_t i = 0; i < descriptor.ms_methods.size(); i++)
		{
			behaviac::CMethodBase* method = descriptor.ms_methods[i];

			if (method->GetID() == name)
			{
				method->GetParams(out_param);
				return;
			}
		}
	}
#endif

	//behaviac::CMethodBase* behaviac::CTagObject::GetMethod(const behaviac::CStringCRC& functionName)
	//{
	//    const CTagObjectDescriptor& descriptor = GetDescriptor();
	//    for(size_t i=0;i<descriptor.ms_methods.size();i++)
	//    {
	//        behaviac::CMethodBase* method = descriptor.ms_methods[i];
	//        if(method->GetID() == functionName)
	//        {
	//            return method;
	//        }
	//    }
	//    return NULL;
	//}
	static EnumClassMap_t* s_enumClasses;
	EnumClassMap_t& GetEnumValueNameMaps()
	{
		if (!s_enumClasses)
		{
			s_enumClasses = BEHAVIAC_NEW EnumClassMap_t;
		}

		BEHAVIAC_ASSERT(s_enumClasses);
		return *s_enumClasses;
	}

	void CleanupEnumValueNameMaps()
	{
		if (s_enumClasses)
		{
			EnumClassMap_t* enumClasses = s_enumClasses;

			for (EnumClassMap_t::iterator it = enumClasses->begin(); it != enumClasses->end(); ++it)
			{
				EnumClassDescriptionBSS_t* pEnumClassD = (EnumClassDescriptionBSS_t*)it->second;
				pEnumClassD->descriptor->valueMaps.clear();
				BEHAVIAC_DELETE(pEnumClassD->descriptor);
				pEnumClassD->descriptor = 0;
			}

			s_enumClasses->clear();
			BEHAVIAC_DELETE(s_enumClasses);
			s_enumClasses = 0;
		}
	}

	//const EnumValueNameMap_t* GetEnumClassValueNames(const char* enumClassName)
	//{
	//	EnumClassMap_t::iterator it = gs_enumClasses.find(enumClassName);
	//	if (it != gs_enumClasses.end())
	//	{
	//		const EnumValueNameMap_t* pValues = it->second;
	//
	//		return pValues;
	//	}
	//
	//	return 0;
	//}

	bool Equal_Struct(const CTagObjectDescriptor& object_desc, const behaviac::CTagObject* lhs, const behaviac::CTagObject* rhs)
	{
		CTagObjectDescriptor::MembersVector_t::const_iterator it = object_desc.ms_members.membersVector.begin();
		CTagObjectDescriptor::MembersVector_t::const_iterator itEnd = object_desc.ms_members.membersVector.end();

		for (; it != itEnd; ++it)
		{
			behaviac::IMemberBase* m = *it;

			bool bEqual = m->Equal(lhs, rhs);

			if (!bEqual)
			{
				return false;
			}
		}

		return true;
	}
}

namespace behaviac
{
    namespace StringUtils
    {
		//it returns true if 'str' starts with a count followed by ':'
		//3:{....}
		bool IsArrayString(const behaviac::string& str, int posStart, behaviac::string::size_type& posEnd)
		{
			//begin of the count of an array?
			//int posStartOld = posStart;

			bool bIsDigit = false;

			int strLen = (int)str.size();
			while (posStart < strLen)
			{
				char c = str[posStart++];

				if (isdigit(c))
				{
					bIsDigit = true;
				}
				else if (c == ':' && bIsDigit)
				{
					//transit_points = 3:{coordX = 0; coordY = 0; } | {coordX = 0; coordY = 0; } | {coordX = 0; coordY = 0; };
					//skip array item which is possible a struct
					int depth = 0;
					for (int posStart2 = posStart; posStart2 < strLen; posStart2++)
					{
						char c1 = str[posStart2];

						if (c1 == ';' && depth == 0)
						{
							//the last ';'
							posEnd = posStart2;
							break;
						}
						else if (c1 == '{')
						{
							BEHAVIAC_ASSERT(depth < 10);
							depth++;
						}
						else if (c1 == '}')
						{
							BEHAVIAC_ASSERT(depth > 0);
							depth--;
						}
					}
					
					return true;
				}
				else
				{
					break;
				}
			}

			return false;
		}

        XmlNodeRef MakeXmlNodeStruct(const char* str, const behaviac::string& typeNameT)
        {
            behaviac::string src = str;

            //{color=0;id=;type={bLive=false;name=0;weight=0;};}
            //the first char is '{'
            //the last char is '}'
            behaviac::string::size_type posCloseBrackets = behaviac::StringUtils::Private::SkipPairedBrackets(src);
            BEHAVIAC_ASSERT(posCloseBrackets != behaviac::string::npos);

            bool bIsStructMember = false;
            XmlNodeRef xmlNode = CreateXmlNode(typeNameT.c_str());

            //{color=0;id=;type={bLive=false;name=0;weight=0;};}
			//{color=0;id=;type={bLive=false;name=0;weight=0;};transit_points=3:{coordX=0;coordY=0;}|{coordX=0;coordY=0;}|{coordX=0;coordY=0;};}
            behaviac::string::size_type posBegin = 1;
            behaviac::string::size_type posEnd = src.find_first_of(';', posBegin);

            while (posEnd != behaviac::string::npos)
            {
                BEHAVIAC_ASSERT(src[posEnd] == ';');

                //the last one might be empty
                if (posEnd > posBegin)
                {
                    behaviac::string::size_type posEqual = src.find_first_of('=', posBegin);
                    BEHAVIAC_ASSERT(posEqual > posBegin);

					size_t length = posEqual - posBegin;
                    behaviac::string memmberName = src.substr(posBegin, length);
                    behaviac::string memmberValue;
                    char c = src[posEqual + 1];

                    if (c != '{')
                    {
						//to check if it is an array
						if (IsArrayString(src, posEqual + 1, posEnd))
						{
							length = posEnd - posEqual - 1;
							memmberValue = src.substr(posEqual + 1, length);
						}
						else
						{
							length = posEnd - posEqual - 1;
							memmberValue = src.substr(posEqual + 1, length);
						}
                    }
                    else
                    {
                        bIsStructMember = true;

                        const char* pStructBegin = src.c_str();
                        pStructBegin += posEqual + 1;
                        const char* posCloseBrackets_ = behaviac::StringUtils::Private::SkipPairedBrackets(pStructBegin);
                        length = posCloseBrackets_ - pStructBegin + 1;

                        memmberValue = src.substr(posEqual + 1, length);

                        posEnd = posEqual + 1 + length;
                    }

                    if (bIsStructMember)
                    {
                        XmlNodeRef memberNode = MakeXmlNodeStruct(memmberValue.c_str(), memmberName);

                        xmlNode->addChild(memberNode);
                    }
                    else
                    {
                        //behaviac::string memmberNameFull = typeNameT + "::" + memmberName;
                        //xmlNode->setAttr(memmberNameFull.c_str(), memmberValue.c_str());
                        xmlNode->setAttr(memmberName.c_str(), memmberValue.c_str());
                    }
                }

                bIsStructMember = false;

                //skip ';'
                posBegin = posEnd + 1;

                //{color=0;id=;type={bLive=false;name=0;weight=0;};transit_points=3:{coordX=0;coordY=0;}|{coordX=0;coordY=0;}|{coordX=0;coordY=0;};}
                posEnd = src.find_first_of(';', posBegin);

                if (posEnd > posCloseBrackets)
                {
                    break;
                }
            }

            return xmlNode;
        }

        bool MakeStringFromXmlNodeStruct(behaviac::XmlConstNodeRef xmlNode, behaviac::string& result)
        {
            //xmlNode->getXML(result);
            result = "{";

            for (int a = 0; a < xmlNode->getAttrCount(); ++a)
            {
                const char* tag = xmlNode->getAttrTag(a);
                const char* value = xmlNode->getAttr(a);

				char temp[1024];
				string_sprintf(temp, "%s=%s;", tag, value);
				result += temp;
            }

            for (int c = 0; c < xmlNode->getChildCount(); ++c)
            {
                behaviac::XmlConstNodeRef childNode = xmlNode->getChild(c);

                behaviac::string childString;

                if (MakeStringFromXmlNodeStruct(childNode, childString))
                {
                    result += childString;
                    result += ";";
                }
            }

            result += "}";

            return true;
        }

        bool ParseForStruct(const char* str, behaviac::string& strT, behaviac::map<behaviac::CStringCRC, behaviac::IInstanceMember*>  &props)
        {
            const char* pB = str;

            while (*str)
            {
                char c = *str;

                if (c == ';' || c == '{' || c == '}')
                {
                    const char* p = pB;

                    while (p <= str)
                    {
                        strT += *p++;
                    }

                    pB = str + 1;

                }
                else if (c == ' ')
                {
                    //par or property
                    behaviac::string propName;
                    const char* p = pB;

                    while (*p != '=')
                    {
                        propName += *p++;
                    }

                    //skip '='
                    BEHAVIAC_ASSERT(*p == '=');
                    p++;

                    behaviac::string typeStr;

                    while (*p != ' ')
                    {
                        typeStr += *p++;
                    }

                    bool bStatic = false;

                    if (typeStr == "static")
                    {
                        //skip ' '
                        BEHAVIAC_ASSERT(*p == ' ');
                        p++;

                        while (*p != ' ')
                        {
                            typeStr += *p++;
                        }

                        bStatic = true;
                    }

                    BEHAVIAC_UNUSED_VAR(bStatic);


                    behaviac::string parName;

                    //skip ' '
                    BEHAVIAC_ASSERT(*str == ' ');
                    str++;

                    while (*str != ';')
                    {
                        parName += *str++;
                    }

                    behaviac::CStringCRC propertyId(propName.c_str());
                    //props[propertyId] = behaviac::Property::Create(typeStr.c_str(), parName.c_str(), bStatic, 0);
                    BEHAVIAC_ASSERT(false);

                    //skip ';'
                    BEHAVIAC_ASSERT(*str == ';');

                    pB = str + 1;
                }

                str++;
            }

            return true;
        }
    }//namespace StringUtils
}
