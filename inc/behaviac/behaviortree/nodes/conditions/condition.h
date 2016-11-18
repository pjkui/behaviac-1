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

#ifndef _BEHAVIAC_BEHAVIORTREE_CONDITION_H_
#define _BEHAVIAC_BEHAVIORTREE_CONDITION_H_

#include "behaviac/common/base.h"
#include "behaviac/behaviortree/behaviortree.h"
#include "behaviac/behaviortree/behaviortree_task.h"
#include "behaviac/property/property.h"

#include "behaviac/common/member.h"

#include "behaviac/behaviortree/nodes/conditions/conditionbase.h"

namespace behaviac
{
    /*! \addtogroup treeNodes Behavior Tree
    * @{
    * \addtogroup Condition
    * @{ */

    /**
    Condition node can compare the value of right and left. return Failure or Success
    */

    class BEHAVIAC_API Condition : public ConditionBase
    {
    private:
        template<typename T>
        static bool Register(const char* typeName)
        {

#if BEHAVIAC_UNUSED_CODE
            ComparatorCreators()[typeName] = &_Creator<T>;
#endif //BEHAVIAC_UNUSED_CODE


            return true;
        }

        template<typename T>
        static void UnRegister(const char* typeName)
        {
#if BEHAVIAC_UNUSED_CODE
            ComparatorCreators().erase(typeName);
#endif //BEHAVIAC_UNUSED_CODE
        }
    public:
#if BEHAVIAC_UNUSED_CODE
        static VariableComparator* Create(const char* typeName, const char* comparionOperator, IInstanceMember* lhs, behaviac::CMethodBase* lhs_m, IInstanceMember* rhs, behaviac::CMethodBase* rhs_m);
#endif //BEHAVIAC_UNUSED_CODE

        static void Cleanup();

    private:
#if BEHAVIAC_UNUSED_CODE

        template<typename T>
        static VariableComparator* _Creator(E_VariableComparisonType comparisonType,
            const IInstanceMember* lhs, const behaviac::CMethodBase* lhs_m, const IInstanceMember* rhs, const behaviac::CMethodBase* rhs_m)
        {
            typedef VariableComparatorImpl<T> VariableComparatorType;
            VariableComparatorType* pComparator = BEHAVIAC_NEW VariableComparatorType;
            BEHAVIAC_ASSERT(false);

            return pComparator;
        }

        typedef VariableComparator* VariableComparatorCreator(E_VariableComparisonType comparisonType, const IInstanceMember* lhs, const behaviac::CMethodBase* lhs_m, const IInstanceMember* rhs, const behaviac::CMethodBase* rhs_m);
        typedef behaviac::map<behaviac::string, VariableComparatorCreator*> VariableComparators;
        typedef VariableComparators::iterator VariableComparatorIterator;
        static VariableComparators* ms_comparatorCreators;//TODO: 3: remove this line and relative lines
        static VariableComparators& ComparatorCreators();
#endif //BEHAVIAC_UNUSED_CODE

    public:
        BEHAVIAC_DECLARE_DYNAMIC_TYPE(Condition, ConditionBase);

        Condition();
        virtual ~Condition();
        virtual void load(int version, const char* agentType, const properties_t& properties);
        virtual bool Evaluate(Agent* pAgent);

    protected:
        virtual bool IsValid(Agent* pAgent, BehaviorTask* pTask) const;

    private:
        virtual BehaviorTask* createTask() const;

    protected:
        IInstanceMember*			m_opl;
        IInstanceMember*			m_opr;
        EOperatorType               m_operator;
        friend class ConditionTask;
    };

    class BEHAVIAC_API ConditionTask : public ConditionBaseTask
    {
    public:
        BEHAVIAC_DECLARE_DYNAMIC_TYPE(ConditionTask, ConditionBaseTask);

        ConditionTask() : ConditionBaseTask()
        {
        }

        virtual ~ConditionTask()
        {
        }

    protected:
        virtual void copyto(BehaviorTask* target) const;
        virtual void save(IIONode* node) const;
        virtual void load(IIONode* node);

        virtual bool onenter(Agent* pAgent);
        virtual void onexit(Agent* pAgent, EBTStatus s);
        virtual EBTStatus update(Agent* pAgent, EBTStatus childStatus);
    };
    /*! @} */
    /*! @} */
}

#endif//_BEHAVIAC_BEHAVIORTREE_CONDITION_H_
