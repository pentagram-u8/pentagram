/*
 *  p_dynamic_cast.cpp - Pentagram Dynamic Cast Emulation Header
 *
 *  Copyright (C) 2003  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef	P_DYNAMIC_CAST_H
#define	P_DYNAMIC_CAST_H

// The Pentagram dynamic cast
template<class A, class B> inline A p_dynamic_cast (B *object)
{
	return (A) object->DoDynamicCast(& (((A)0)->ClassTypeConstant) );
}

// This is just a 'type' used to differentiate each class.
struct RunTimeClassType
{
	char	*class_name;
};

//
// Include this in every class. It sets up the class to be able to use 
// p_dynamic_cast. Make sure this is public!
//
#define ENABLE_DYNAMIC_CAST(Classname)							\
	const static RunTimeClassType	ClassTypeConstant;			\
	virtual void * DoDynamicCast(const RunTimeClassType *type);


//
// Define this in the source files of base classes
//
#define DEFINE_DYNAMIC_CAST_CODE_BASE_CLASS(Classname)			\
const RunTimeClassType Classname::ClassTypeConstant = {			\
	#Classname													\
};																\
																\
void * Classname::DoDynamicCast(const RunTimeClassType *type)	\
{																\
	if (type == &ClassTypeConstant) return this;				\
	return 0;													\
}																\


//
// Define this in the source files of child classes, with 1 parent
//
#define DEFINE_DYNAMIC_CAST_CODE(Classname,ParentClassname)		\
const RunTimeClassType Classname::ClassTypeConstant = {			\
	#Classname													\
};																\
																\
void * Classname::DoDynamicCast(const RunTimeClassType *type)	\
{																\
	if (type == &ClassTypeConstant) return this;				\
	return ParentClassname::DoDynamicCast(type);				\
}



//
// Define this in the source files of child classes, with 2 parents
//
#define DEFINE_DYNAMIC_CAST_CODE_MULTI2(Classname,Parent1,Parent2)	\
const RunTimeClassType Classname::ClassTypeConstant = {				\
	#Classname														\
};																	\
																	\
void * Classname::DoDynamicCast(const RunTimeClassType *type)		\
{																	\
	if (type == &ClassTypeConstant) return this;					\
	void *ret = Parent1::DoDynamicCast(type);						\
	if (ret) return ret;											\
	return Parent2::DoDynamicCast(type);							\
}


#endif //P_DYNAMIC_CAST
