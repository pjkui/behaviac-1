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

#ifndef _BEHAVIAC_EXTENDREFTYPE_H_
#define _BEHAVIAC_EXTENDREFTYPE_H_

#include "behaviac/common/rttibase.h"

#include "behaviac/common/string/fromstring.h"
#include "behaviac/common/string/tostring.h"

#include "behaviac/common/basictypes.h"
#include "behaviac/common/rttibase.h"

#include "behaviac/common/object/tagobject.h"
#include "behaviac/agent/agent.h"

//////////////////////////////////////////////////////////////////////////
//we declare a type "Node" to simulate a type defined in a thirdparty lib
namespace TestNS
{
	struct Node
	{
        behaviac::string name;
	};
}

//////////////////////////////////////////////////////////////////////////
//
struct myNode
{
	DECLARE_BEHAVIAC_STRUCT(myNode, true);

	myNode()
	{}

	myNode(const TestNS::Node& v)
	{
		BEHAVIAC_UNUSED_VAR(v);
	}

	behaviac::string name;
};

BEHAVIAC_EXTEND_EXISTING_TYPE(myNode, TestNS::Node);

namespace behaviac
{
	// ValueToString & ValueFromString
	namespace StringUtils
	{
		namespace Private
		{
			template<>
			inline behaviac::string ToString(const TestNS::Node& val)
			{
				//myNode::ToString is defined by DECLARE_BEHAVIAC_STRUCT(myNode)
				myNode temp(val);
				return temp.ToString();
			}

			template<>
			inline bool ParseString(const char* str, TestNS::Node& val)
			{
				BEHAVIAC_UNUSED_VAR(val);

				myNode temp;

				//myNode::ParseString is defined by DECLARE_BEHAVIAC_STRUCT(myNode)
				if (temp.ParseString(str))
				{
					return true;
				}

				return false;
			}
		}
	}
}

// SwapByteImplement helpers
template< typename SWAPPER >
inline void SwapByteImplement(TestNS::Node& v)
{
}

//operators
namespace behaviac
{
	namespace PrivateDetails
	{
		//------------------------------------------------------------------------
		template<>
		inline bool Equal(const TestNS::Node& lhs, const TestNS::Node& rhs)
		{
			return &lhs == &rhs;
		}
	}
}

//add the following to a cpp
//BEHAVIAC_BEGIN_STRUCT(myNode)
//{
//	BEHAVIAC_STRUCT_DISPLAYNAME(L"")
//	BEHAVIAC_STRUCT_DESC(L"")
//
//	BEHAVIAC_REGISTER_STRUCT_PROPERTY(x);
//	BEHAVIAC_REGISTER_STRUCT_PROPERTY(y);
//}
//BEHAVIAC_END_STRUCT()


//add the following to register/unregister
//behaviac::TypeRegister::Register<TestNS::Node>("TestNS::Node");
//behaviac::TypeRegister::UnRegister<TestNS::Node>("TestNS::Node");

#endif//_BEHAVIAC_EXTENDREFTYPE_H_
