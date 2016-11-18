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
#include "behaviac/common/operation.h"
#include "behaviac/common/rttibase.h"


namespace behaviac
{
#if BEHAVIAC_UNUSED_CODE

    behaviac::map<behaviac::string, IICompareValue*> ComparerRegister::ms_valueComparers;
    void ComparerRegister::Register()
    {
        //CompareValueBool* pComparer = new CompareValueBool();
        //ms_valueComparers[GetTypeDescString<bool>()] = pComparer;
        ms_valueComparers[GetTypeDescString<bool>()] = BEHAVIAC_NEW CompareValueBool();

        //CompareValueInt* pComparer = new CompareValueInt();
        //ms_valueComparers[GetTypeDescString<int>()] = pComparer;
        ms_valueComparers[GetTypeDescString<int>()] = BEHAVIAC_NEW CompareValueInt();

        //CompareValueLong* pComparer = new CompareValueLong();
        //ms_valueComparers[GetTypeDescString<long>()] = pComparer;
        ms_valueComparers[GetTypeDescString<long>()] = BEHAVIAC_NEW CompareValueLong();

        //CompareValueShort* pComparer = new CompareValueShort();
        //ms_valueComparers[GetTypeDescString<short>()] = pComparer;
        ms_valueComparers[GetTypeDescString<short>()] = BEHAVIAC_NEW CompareValueShort();

        //CompareValueuint32_t* pComparer = new CompareValueuint32_t();
        //ms_valueComparers[GetTypeDescString<uint32_t>()] = pComparer;
        ms_valueComparers[GetTypeDescString<uint32_t>()] = BEHAVIAC_NEW CompareValueuint32_t();

        //CompareValueULong* pComparer = new CompareValueULong();
        //ms_valueComparers[GetTypeDescString<unsigned long>()] = pComparer;
        ms_valueComparers[GetTypeDescString<unsigned long>()] = BEHAVIAC_NEW CompareValueULong();

        //CompareValueUShort* pComparer = new CompareValueUShort();
        //ms_valueComparers[GetTypeDescString<unsigned short>()] = pComparer;
        ms_valueComparers[GetTypeDescString<unsigned short>()] = BEHAVIAC_NEW CompareValueUShort();

        //CompareValueFloat* pComparer = new CompareValueFloat();
        //ms_valueComparers[GetTypeDescString<float>()] = pComparer;
        ms_valueComparers[GetTypeDescString<float>()] = BEHAVIAC_NEW CompareValueFloat();

        //CompareValueDouble* pComparer = new CompareValueDouble();
        //ms_valueComparers[GetTypeDescString<double>()] = pComparer;
        ms_valueComparers[GetTypeDescString<double>()] = BEHAVIAC_NEW CompareValueDouble();

        //CompareValueString* pComparer = new CompareValueString();
        //ms_valueComparers[GetTypeDescString<string>()] = pComparer;
        ms_valueComparers[GetTypeDescString<string>()] = BEHAVIAC_NEW CompareValueString();

        //CompareValueObject* pComparer = new CompareValueObject();
        //ms_valueComparers[GetTypeDescString<void*>()] = pComparer;
        ms_valueComparers[GetTypeDescString<void*>()] = BEHAVIAC_NEW CompareValueObject();
    }
    void ComparerRegister::Init()
    {
        if (ms_valueComparers.size() == 0)
        {
            Register();
        }
    }
    void ComparerRegister::Cleanup()
    {
        for (behaviac::map<behaviac::string, IICompareValue*>::iterator it = ms_valueComparers.begin(); it != ms_valueComparers.end(); ++it)
        {
            IICompareValue* p = it->second;
            BEHAVIAC_DELETE p;
        }

        ms_valueComparers.clear();
    }

    IICompareValue* ComparerRegister::Get(behaviac::string type)
    {
        if (ms_valueComparers.find(type) != ms_valueComparers.end())
        {
            IICompareValue* pComparer = ms_valueComparers[type];
            return pComparer;
        }

        return NULL;
    }
#endif 

    behaviac::map<behaviac::string, IIComputeValue*> ComputerRegister::ms_valueComputers;

    void ComputerRegister::Register()
    {
        ms_valueComputers[GetTypeDescString<int>()] = BEHAVIAC_NEW ComputeValueInt();
        ms_valueComputers[GetTypeDescString<long>()] = BEHAVIAC_NEW ComputeValueLong();
        ms_valueComputers[GetTypeDescString<short>()] = BEHAVIAC_NEW ComputeValueShort();
        ms_valueComputers[GetTypeDescString<char>()] = BEHAVIAC_NEW ComputeValueByte();
        ms_valueComputers[GetTypeDescString<uint32_t>()] = BEHAVIAC_NEW ComputeValueuint32_t();
        ms_valueComputers[GetTypeDescString<unsigned long>()] = BEHAVIAC_NEW ComputeValueULong();
        ms_valueComputers[GetTypeDescString<unsigned short>()] = BEHAVIAC_NEW ComputeValueUShort();
        ms_valueComputers[GetTypeDescString<unsigned char>()] = BEHAVIAC_NEW ComputeValueUByte();
        ms_valueComputers[GetTypeDescString<float>()] = BEHAVIAC_NEW ComputeValueFloat();
        ms_valueComputers[GetTypeDescString<double>()] = BEHAVIAC_NEW ComputeValueDouble();

    }
    void ComputerRegister::Init()
    {
        if (ms_valueComputers.size() == 0)
        {
            Register();
        }
    }
    void ComputerRegister::Cleanup()
    {
        for (behaviac::map<behaviac::string, IIComputeValue*>::iterator it = ms_valueComputers.begin(); it != ms_valueComputers.end(); ++it)
        {
            IIComputeValue* p = it->second;
            BEHAVIAC_DELETE p;
        }
        ms_valueComputers.clear();
    }
    IIComputeValue* ComputerRegister::Get(behaviac::string type)
    {
        if (ms_valueComputers.find(type) != ms_valueComputers.end())
        {
            IIComputeValue* pComparer = ms_valueComputers[type];
            return pComparer;
        }

        BEHAVIAC_ASSERT(false);

        return NULL;
    }

}
