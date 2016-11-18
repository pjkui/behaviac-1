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

#ifndef _BEHAVIAC_COMMON_OPERATION_H_
#define _BEHAVIAC_COMMON_OPERATION_H_

#include "behaviac/common/config.h"
#include "behaviac/common/assert.h"

#include "behaviac/common/base.h"
#include "behaviac/common/string/stringutils.h"
#include "behaviac/property/operators.inl"


#include "behaviac/common/rttibase.h"

namespace behaviac
{

    class FieldInfo;
    //enum EOperatorType
    //{
    //    E_INVALID,
    //    E_ASSIGN,        // =
    //    E_ADD,           // +
    //    E_SUB,           // -
    //    E_MUL,           // *
    //    E_DIV,           // /
    //    E_EQUAL,         // ==
    //    E_NOTEQUAL,      // !=
    //    E_GREATER,       // >
    //    E_LESS,          // <
    //    E_GREATEREQUAL,  // >=
    //    E_LESSEQUAL      // <=
    //};
    
    template<typename T>
    class Operator;

    class OperationUtils
    {
    public:
        static EOperatorType ParseOperatorType(const char* operatorType)
        {
            if (StringUtils::StringEqual(operatorType, "Invalid"))
            {
                return E_INVALID;
            }
            else if (StringUtils::StringEqual(operatorType, "Assign"))
            {
                return E_ASSIGN;
            }
            else if (StringUtils::StringEqual(operatorType, "Add"))
            {
                return E_ADD;
            }
            else if (StringUtils::StringEqual(operatorType, "Sub"))
            {
                return E_SUB;
            }
            else if (StringUtils::StringEqual(operatorType, "Mul"))
            {
                return E_MUL;
            }
            else if (StringUtils::StringEqual(operatorType, "Div"))
            {
                return E_DIV;
            }
            else if (StringUtils::StringEqual(operatorType, "Equal"))
            {
                return E_EQUAL;
            }
            else if (StringUtils::StringEqual(operatorType, "NotEqual"))
            {
                return E_NOTEQUAL;
            }
            else if (StringUtils::StringEqual(operatorType, "Greater"))
            {
                return E_GREATER;
            }
            else if (StringUtils::StringEqual(operatorType, "Less"))
            {
                return E_LESS;
            }
            else if (StringUtils::StringEqual(operatorType, "GreaterEqual"))
            {
                return E_GREATEREQUAL;
            }
            else if (StringUtils::StringEqual(operatorType, "LessEqual"))
            {
                return E_LESSEQUAL;
            }

            BEHAVIAC_ASSERT(false);
            return E_INVALID;
        }

    public:
        template<typename T>
        static bool Compare(T left, T right, EOperatorType comparisonType)
        {

            switch (comparisonType)
            {
			case E_EQUAL:        return behaviac::PrivateDetails::Equal(left, right); break;
			case E_NOTEQUAL:     return !behaviac::PrivateDetails::Equal(left, right); break;
			case E_GREATER:      return behaviac::PrivateDetails::Greater(left, right); break;
			case E_GREATEREQUAL: return behaviac::PrivateDetails::GreaterEqual(left, right); break;
			case E_LESS:         return behaviac::PrivateDetails::Less(left, right); break;
			case E_LESSEQUAL:    return behaviac::PrivateDetails::LessEqual(left, right); break;
            default:
                BEHAVIAC_ASSERT(false);
                break;
            }
            BEHAVIAC_ASSERT(false);
            return false;
        }
    public:
        template<typename T>
        static T Compute(T left, T right, EOperatorType computeType)
        {
            if (Meta::IsAgent<T>::Result == 0
                && Meta::IsClass<T>::Result == 0
                && Meta::IsFunction<T>::Result == 0
                && Meta::IsArray<T>::Result == 0
                && Meta::IsPtr<T>::Result == 0)
            {
                if (Meta::IsEnum<T>::Result == 1
                    || Meta::IsVector<T>::Result == 1)
                {
                    BEHAVIAC_ASSERT(false);
                }
                else
                {
                    switch (computeType)
                    {
                    case E_ADD: return Operator<T>::Add(left, right);break;
                    case E_SUB: return Operator<T>::Subtract(left, right);break;
                    case E_MUL: return Operator<T>::Multiply(left, right);break;
                    case E_DIV: return Operator<T>::Divide(left, right);break;
                    default:
                        BEHAVIAC_ASSERT(false);

                        break;
                    }
                }
            }

            BEHAVIAC_ASSERT(false);
            return left;
        }
    };

#if BEHAVIAC_UNUSED_CODE
    class IICompareValue
    {
    public:
        virtual bool ItemEqual(IList* ll, IList* rl, int index)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool MemberEqual(void* lo, void* ro)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }
        virtual ~IICompareValue(){}
    };

    template<typename T>
    class ICompareValue : public IICompareValue
    {
    public:
        virtual bool ItemEqual(IList* ll, IList* rl, int index)
        {
            behaviac::vector<T>* llt = (behaviac::vector<T>*)ll;
            behaviac::vector<T>* rlt = (behaviac::vector<T>*)rl;

            T l = (*llt)[index];
            T r = (*rlt)[index];

            return OperationUtils::Compare<T>(l, r, E_EQUAL);
        }

    public:
        virtual bool MemberEqual(void* lo, void* ro)
        {
            //possible boxing
            T* l = (T*)(lo);
            T* r = (T*)(ro);

            return OperationUtils::Compare<T>(*l, *r, E_EQUAL);
        }

    public:
        virtual bool Equal(T lhs, T rhs) { return false; }

        virtual bool NotEqual(T lhs, T rhs) { return false; }

        virtual bool Greater(T lhs, T rhs) { return false; }

        virtual bool GreaterEqual(T lhs, T rhs) { return false; }

        virtual bool Less(T lhs, T rhs) { return false; }

        virtual bool LessEqual(T lhs, T rhs) { return false; }
    };

    class CompareValueBool : public ICompareValue<bool>
    {
    public:
        virtual bool Equal(bool lhs, bool rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(bool lhs, bool rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(bool lhs, bool rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool GreaterEqual(bool lhs, bool rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool Less(bool lhs, bool rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool LessEqual(bool lhs, bool rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

    };
    class CompareValueInt :public ICompareValue<int>
    {
    public:
        virtual bool Equal(int lhs, int rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(int lhs, int rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(int lhs, int rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(int lhs, int rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(int lhs, int rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(int lhs, int rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueLong :public ICompareValue<long>
    {

    public:
        virtual bool Equal(long lhs, long rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(long lhs, long rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(long lhs, long rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(long lhs, long rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(long lhs, long rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(long lhs, long rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueShort :public ICompareValue<short>
    {
    public:
        virtual bool Equal(short lhs, short rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(short lhs, short rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(short lhs, short rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(short lhs, short rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(short lhs, short rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(short lhs, short rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueuint32_t :public ICompareValue<uint32_t>
    {
    public:
        virtual bool Equal(uint32_t lhs, uint32_t rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(uint32_t lhs, uint32_t rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(uint32_t lhs, uint32_t rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(uint32_t lhs, uint32_t rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(uint32_t lhs, uint32_t rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(uint32_t lhs, uint32_t rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueULong :public ICompareValue<unsigned long>
    {
    public:
        virtual bool Equal(unsigned long lhs, unsigned long rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(unsigned long lhs, unsigned long rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(unsigned long lhs, unsigned long rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(unsigned long lhs, unsigned long rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(unsigned long lhs, unsigned long rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(unsigned long lhs, unsigned long rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueUShort :public ICompareValue<unsigned short>
    {
    public:
        virtual bool Equal(unsigned short lhs, unsigned short rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(unsigned short lhs, unsigned short rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(unsigned short lhs, unsigned short rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(unsigned short lhs, unsigned short rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(unsigned short lhs, unsigned short rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(unsigned short lhs, unsigned short rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueUByte :public ICompareValue<unsigned char>
    {
    public:
        virtual bool Equal(unsigned char lhs, unsigned char rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(unsigned char lhs, unsigned char rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(unsigned char lhs, unsigned char rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(unsigned char lhs, unsigned char rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(unsigned char lhs, unsigned char rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(unsigned char lhs, unsigned char rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueFloat :public ICompareValue<float>
    {
    public:
        virtual bool Equal(float lhs, float rhs)
        {
            return lhs == rhs;
        }

        virtual bool NotEqual(float lhs, float rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(float lhs, float rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(float lhs, float rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(float lhs, float rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(float lhs, float rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueDouble :public ICompareValue<double>
    {
    public:
        virtual bool Equal(double lhs, double rhs)
        {
            return lhs == rhs;
        }
        virtual bool NotEqual(double lhs, double rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(double lhs, double rhs)
        {
            return (lhs > rhs);
        }

        virtual bool GreaterEqual(double lhs, double rhs)
        {
            return (lhs >= rhs);
        }

        virtual bool Less(double lhs, double rhs)
        {
            return (lhs < rhs);
        }

        virtual bool LessEqual(double lhs, double rhs)
        {
            return (lhs <= rhs);
        }

    };

    class CompareValueString :public ICompareValue<behaviac::string>
    {
    public:
        CompareValueString()
        {

        }

    public:
        virtual bool Equal(behaviac::string lhs, behaviac::string rhs)
        {
            return lhs == rhs;
        }

        virtual bool NotEqual(behaviac::string lhs, behaviac::string rhs)
        {
            return lhs != rhs;
        }

        virtual bool Greater(behaviac::string lhs, behaviac::string rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool GreaterEqual(behaviac::string lhs, behaviac::string rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool Less(behaviac::string lhs, behaviac::string rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool LessEqual(behaviac::string lhs, behaviac::string rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

    };

    class CompareValueObject :public ICompareValue<void *>
    {
    public:
        virtual bool Equal(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return lhs == rhs;
        }

        virtual bool NotEqual(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return lhs != rhs;
        }

        virtual bool Greater(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool GreaterEqual(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool Less(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

        virtual bool LessEqual(void* lhs, void* rhs)
        {
            BEHAVIAC_ASSERT(false);
            return false;
        }

    };

    class BEHAVIAC_API ComparerRegister
    {
    private:
        static behaviac::map<behaviac::string, IICompareValue*> ms_valueComparers;

    private:
        static void Register();

    public:
        static void Init();
        static void Cleanup();
        static IICompareValue* Get(behaviac::string type);

        template<typename T>
        static ICompareValue<T>* Get()
        {
            behaviac::string type = GetTypeDescString<T>();
            return (ICompareValue<T>*)Get(type);
        }

    };
#endif

    ///
    class IIComputeValue
    {
    public:
        virtual ~IIComputeValue()
        {

        }
    };

    template<typename T>
    class IComputeValue : public IIComputeValue
    {
    public:
        virtual T Add(T opr1, T opr2) = 0;

        virtual T Sub(T opr1, T opr2) = 0;

        virtual T Mul(T opr1, T opr2) = 0;

        virtual T Div(T opr1, T opr2) = 0;
        virtual ~IComputeValue()
        {

        }

    };

    class ComputeValueInt : public IComputeValue<int>
    {
    public:
        int Add(int lhs, int rhs)
        {
            int r = (lhs + rhs);
            return r;
        }

        int Sub(int lhs, int rhs)
        {
            int r = (lhs - rhs);
            return r;
        }

        int Mul(int lhs, int rhs)
        {
            int r = (lhs * rhs);
            return r;
        }

        int Div(int lhs, int rhs)
        {
            int r = (lhs / rhs);
            return r;
        }

    };

    class ComputeValueLong : public IComputeValue<long>
    {
    public:
        long Add(long lhs, long rhs)
        {
            long r = (lhs + rhs);
            return r;
        }

        long Sub(long lhs, long rhs)
        {
            long r = (lhs - rhs);
            return r;
        }

        long Mul(long lhs, long rhs)
        {
            long r = (lhs * rhs);
            return r;
        }

        long Div(long lhs, long rhs)
        {
            long r = (lhs / rhs);
            return r;
        }

    };

    class ComputeValueShort : public IComputeValue<short>
    {
    public:
        short Add(short lhs, short rhs)
        {
            short r = (short)(lhs + rhs);
            return r;
        }

        short Sub(short lhs, short rhs)
        {
            short r = (short)(lhs - rhs);
            return r;
        }

        short Mul(short lhs, short rhs)
        {
            short r = (short)(lhs * rhs);
            return r;
        }

        short Div(short lhs, short rhs)
        {
            short r = (short)(lhs / rhs);
            return r;
        }

    };

    class ComputeValueByte : public IComputeValue<char>
    {
        char Add(char lhs, char rhs)
        {
            char r = (char)(lhs + rhs);
            return r;
        }

        char Sub(char lhs, char rhs)
        {
            char r = (char)(lhs - rhs);
            return r;
        }

        char Mul(char lhs, char rhs)
        {
            char r = (char)(lhs * rhs);
            return r;
        }

        char Div(char lhs, char rhs)
        {
            char r = (char)(lhs / rhs);
            return r;
        }

    };

    class ComputeValueuint32_t : public IComputeValue<uint32_t>
    {
    public:
        uint32_t Add(uint32_t lhs, uint32_t rhs)
        {
            uint32_t r = (lhs + rhs);
            return r;
        }

        uint32_t Sub(uint32_t lhs, uint32_t rhs)
        {
            uint32_t r = (lhs - rhs);
            return r;
        }

        uint32_t Mul(uint32_t lhs, uint32_t rhs)
        {
            uint32_t r = (lhs * rhs);
            return r;
        }

        uint32_t Div(uint32_t lhs, uint32_t rhs)
        {
            uint32_t r = (lhs / rhs);
            return r;
        }

    };

    class ComputeValueULong : public IComputeValue<unsigned long>
    {
    public:
        unsigned long Add(unsigned long lhs, unsigned long rhs)
        {
            unsigned long r = (lhs + rhs);
            return r;
        }

        unsigned long Sub(unsigned long lhs, unsigned long rhs)
        {
            unsigned long r = (lhs - rhs);
            return r;
        }

        unsigned long Mul(unsigned long lhs, unsigned long rhs)
        {
            unsigned long r = (lhs * rhs);
            return r;
        }

        unsigned long Div(unsigned long lhs, unsigned long rhs)
        {
            unsigned long r = (lhs / rhs);
            return r;
        }

    };

    class ComputeValueUShort : public IComputeValue<unsigned short>
    {
    public:
        unsigned short Add(unsigned short lhs, unsigned short rhs)
        {
            unsigned short r = (unsigned short)(lhs + rhs);
            return r;
        }

        unsigned short Sub(unsigned short lhs, unsigned short rhs)
        {
            unsigned short r = (unsigned short)(lhs - rhs);
            return r;
        }

        unsigned short Mul(unsigned short lhs, unsigned short rhs)
        {
            unsigned short r = (unsigned short)(lhs * rhs);
            return r;
        }

        unsigned short Div(unsigned short lhs, unsigned short rhs)
        {
            unsigned short r = (unsigned short)(lhs / rhs);
            return r;
        }

    };

    class ComputeValueUByte : public IComputeValue<unsigned char>
    {
    public:
        unsigned char Add(unsigned char lhs, unsigned char rhs)
        {
            unsigned char r = (unsigned char)(lhs + rhs);
            return r;
        }

        unsigned char Sub(unsigned char lhs, unsigned char rhs)
        {
            unsigned char r = (unsigned char)(lhs - rhs);
            return r;
        }

        unsigned char Mul(unsigned char lhs, unsigned char rhs)
        {
            unsigned char r = (unsigned char)(lhs * rhs);
            return r;
        }

        unsigned char Div(unsigned char lhs, unsigned char rhs)
        {
            unsigned char r = (unsigned char)(lhs / rhs);
            return r;
        }

    };

    class ComputeValueFloat : public IComputeValue<float>
    {
    public:
        float Add(float lhs, float rhs)
        {
            float r = (lhs + rhs);
            return r;
        }

        float Sub(float lhs, float rhs)
        {
            float r = (lhs - rhs);
            return r;
        }

        float Mul(float lhs, float rhs)
        {
            float r = (lhs * rhs);
            return r;
        }

        float Div(float lhs, float rhs)
        {
            float r = (lhs / rhs);
            return r;
        }

    };

    class ComputeValueDouble : public IComputeValue<double>
    {
    public:
        double Add(double lhs, double rhs)
        {
            double r = (lhs + rhs);
            return r;
        }

        double Sub(double lhs, double rhs)
        {
            double r = (lhs - rhs);
            return r;
        }

        double Mul(double lhs, double rhs)
        {
            double r = (lhs * rhs);
            return r;
        }

        double Div(double lhs, double rhs)
        {
            double r = (lhs / rhs);
            return r;
        }

    };

    class BEHAVIAC_API ComputerRegister
    {
    private:
        static behaviac::map<behaviac::string, IIComputeValue*> ms_valueComputers;
        static void Register();

    public:
        static void Init();
        static void Cleanup();
        static IIComputeValue* Get(behaviac::string type);

        template<typename T>
        static IComputeValue<T>* Get()
        {
            behaviac::string type = GetTypeDescString<T>();
            return (IComputeValue<T>*)Get(type);
        }
    };

    template<typename T>
    class Operator
    {
    public:
        static T Add(T left, T right)
        {
            IComputeValue<T>* c = ComputerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);

            return c->Add(left, right);
        }

        static T Subtract(T left, T right)
        {
            IComputeValue<T>* c = ComputerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->Sub(left, right);
        }

        static T Multiply(T left, T right)
        {
            IComputeValue<T>* c = ComputerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->Mul(left, right);
        }

        static T Divide(T left, T right)
        {
            IComputeValue<T>* c = ComputerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->Div(left, right);
        }
#if BEHAVIAC_UNUSED_CODE
    private:
        static bool MemberEqual(T left, T right)
        {
            BEHAVIAC_ASSERT(false);
            return left.Equals(right);
        }

    public:
        static bool ClassEqual(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            if (c == NULL)
            {
                BEHAVIAC_ASSERT(false, "class Equal porting from cpp");
                return left.Equals(right);
            }

            return c->Equal(left, right);
        }

        static bool ListEqual(T left, T right)
        {
            BEHAVIAC_ASSERT(false);
            return true;
        }

    public:
        static bool Equal(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            if (c == NULL)
            {
                ICompareValue<T>* co = ComparerRegister::Get<T>();

                return co->Equal(left, right);
            }

            BEHAVIAC_ASSERT(c != NULL);

            return c->Equal(left, right);
        }

        static bool NotEqual(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();

            if (c == NULL)
            {
                ICompareValue<T>* co = ComparerRegister::Get<T>();

                return co->NotEqual(left, right);
            }

            BEHAVIAC_ASSERT(c != NULL);

            return c->NotEqual(left, right);
        }

        static bool GreaterThan(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->Greater(left, right);
        }

        static bool GreaterThanOrEqual(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->GreaterEqual(left, right);
        }

        static bool LessThan(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->Less(left, right);
        }

        static bool LessThanOrEqual(T left, T right)
        {
            ICompareValue<T>* c = ComparerRegister::Get<T>();
            BEHAVIAC_ASSERT(c != NULL);
            return c->LessEqual(left, right);
        }
#endif

    };

}//namespace behaviac
#endif // _BEHAVIAC_COMMON_OPERATION_H_
