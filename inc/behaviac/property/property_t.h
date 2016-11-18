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

#ifndef _BEHAVIAC_PROPERTY_T_H_
#define _BEHAVIAC_PROPERTY_T_H_

#include "behaviac/property/property.h"
#include "behaviac/agent/agent.h"
#include "behaviac/common/object/member.h"
#include "behaviac/common/member.h"

namespace behaviac
{
	class IMemberBase;

    template<typename VariableType>
    inline void ComputeTo(VariableType v, IInstanceMember* pPropertyTo, Agent* pAgentTo, EComputeOperator opr)
    {
        BEHAVIAC_UNUSED_VAR(v);
        BEHAVIAC_UNUSED_VAR(pPropertyTo);
        BEHAVIAC_UNUSED_VAR(pAgentTo);
        BEHAVIAC_UNUSED_VAR(opr);

        BEHAVIAC_ASSERT(0);
    };
}

namespace behaviac
{
    template <typename T>
    const behaviac::IMemberBase* GetMemberFromName(const behaviac::CStringCRC& propertyId)
    {
        const CTagObjectDescriptor& obejctDesc = T::GetObjectDescriptor();

        return obejctDesc.GetMember(propertyId);
    }
}

#endif//_BEHAVIAC_PROPERTY_T_H_
