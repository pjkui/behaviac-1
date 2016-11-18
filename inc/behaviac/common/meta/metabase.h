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

#ifndef _BEHAVIAC_COMMON_META_METABASE_H_
#define _BEHAVIAC_COMMON_META_METABASE_H_

#include "behaviac/common/base.h"
#include "behaviac/common/assert.h"
#include "behaviac/common/staticassert.h"

namespace behaviac
{
	// Meta/Traits

	namespace Meta
	{
		// Empty Class tag
		struct Empty
		{
			typedef Empty Type;
		};

		// Null Class behaviac
		struct Null
		{
			typedef Null Type;
		};

		template< typename T = Null >
		struct Identity
		{
			typedef T Type;
		};

		typedef uint8_t Yes;
		typedef struct
		{
			uint8_t padding[2];
		} No;

		struct TrueType
		{
			enum { Result = true };
		};

		struct FalseType
		{
			enum { Result = false };
		};

		template <bool val>
		struct BooleanType : public FalseType
		{};

		template <>
		struct BooleanType<true> : public TrueType
		{};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Compare two type, Answer is true if they are the same
		template< typename First, typename Second >
		struct IsSame
		{
			enum
			{
				Result = 0
			};
		};

		template< typename First >
		struct IsSame< First, First >
		{
			enum
			{
				Result = 1
			};
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Answer is true if the provided Type is an Array
		template< typename Type >
		struct IsArray
		{
			enum { Result = 0 };
		};

		template< typename Type, int32_t Count >
		struct IsArray< Type[Count] >
		{
			enum { Result = 1 };
		};

		template< typename Type >
		struct IsArray< Type[] >
		{
			enum { Result = 1 };
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Result is true if the provided Type is a class
		template< typename Type >
		struct IsClass
		{
		private:

			template< typename AnswerYes >
			static Yes TYesNoTester(void(AnswerYes::*)(void));

			template< typename AnswerNo >
			static No TYesNoTester(...);

		public:

			enum
			{
				Result = sizeof(TYesNoTester<Type>(0)) == sizeof(Yes)
			};
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Answer is true if the provided type is a reference
		template< typename TTYPE >
		struct IsRef
		{
			enum { Result = 0 };
		};

		template< typename TTYPE >
		struct IsRef< TTYPE& >
		{
			enum { Result = 1 };
		};

		template< typename TTYPE >
		struct IsRef< const TTYPE& >
		{
			enum { Result = 1 };
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Answer is true if the provided type is a pointer
		template< typename TTYPE >
		struct IsPtr
		{
			enum { Result = 0 };
		};

		template< typename TTYPE >
		struct IsPtr< TTYPE* >
		{
			enum { Result = 1 };
		};

		template< typename TTYPE >
		struct IsPtr< const TTYPE* >
		{
			enum { Result = 1 };
		};

		template< typename TTYPE >
		struct IsPtr < TTYPE*& >
		{
			enum { Result = 1 };
		};

		template< typename TTYPE >
		struct IsPtr < const TTYPE*& >
		{
			enum { Result = 1 };
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Answer is true if the provided Type is a pointer or a reference or an array
		template< typename Type >
		struct IsCompound
		{
			enum
			{
				Result = IsPtr< Type >::Result ||
				IsRef< Type >::Result ||
				IsArray< Type >::Result
			};
		};

		///////////////////////////////////////////////////////////////////////////////////////////////
		namespace PrivateDetails
		{
			template <typename BASE, typename DERIVED>
			struct TypeResolver
			{
				template <typename TYPE>
				static Yes ResolvedType(DERIVED const volatile*, TYPE);
				static No  ResolvedType(BASE const volatile*, int);
			};

			template<typename BASE, typename DERIVED>
			struct IsDerivedInternal
			{
				struct TypeCaster
				{
					operator BASE const volatile* () const;
					operator DERIVED const volatile* ();
				};

				static const bool Result = (sizeof(TypeResolver<BASE, DERIVED>::ResolvedType(TypeCaster(), 0)) == sizeof(Yes));
			};
		}

		/// IsDerived<A, B>::Result will be true if type B inherits from type A.
		template<typename BASE, typename DERIVED>
		struct IsDerived : public BooleanType<PrivateDetails::IsDerivedInternal<BASE, DERIVED>::Result>
		{
		};

		///////////////////////////////////////////////////////////////////////////////////////////////        
		// Answer is true only if the provided Type is const
		template< typename T >
		struct IsConst
		{
			enum
			{
				Result = 0
			};
		};

		template< typename T >
		struct IsConst< const T >
		{
			enum { Result = 1 };
		};

		template< typename T >
		struct IsConst< const T& >
		{
			enum { Result = 1 };
		};

		template< typename T >
		struct IsConst< const T* >
		{
			enum { Result = 1 };
		};

		///////////////////////////////////////////////////////////////////////////////////////////////        

		///////////////////////////////////////////////////////////////////////////////////////////////        

		///////////////////////////////////////////////////////////////////////////////////////////////        

		///////////////////////////////////////////////////////////////////////////////////////////////                
	}
}


#endif//_BEHAVIAC_COMMON_META_METABASE_H_
