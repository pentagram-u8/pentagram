/*
Copyright (C) 2003-2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INTRINSICS_H
#define INTRINSICS_H

typedef uint32 (*Intrinsic)(const uint8* args, unsigned int argsize);

#define INTRINSIC(x) static uint32 x (const uint8* args, unsigned int argsize)

// TODO: range checking on args

#define ARG_UINT8(x)   uint8  x = (*args++);
#define ARG_UINT16(x)  uint16 x = (*args++); x += ((*args++) << 8);
#define ARG_UINT32(x)  uint32 x = (*args++); x += ((*args++) << 8); \
                      x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_SINT8(x)   sint8  x = (*args++);
#define ARG_SINT16(x)  sint16 x = (*args++); x += ((*args++) << 8);
#define ARG_SINT32(x)  sint32 x = (*args++); x += ((*args++) << 8); \
                      x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_UC_PTR(x)  uint32 x = (*args++); x += ((*args++) << 8); \
                      x+= ((*args++) << 16); x += ((*args++) << 24);
#define ARG_OBJID(x)  ObjId   x = (*args++); x += ((*args++) << 8);
#define ARG_PROCID(x) ProcId  x = (*args++); x += ((*args++) << 8);

#define ARG_OBJECT_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      Object* x = getObject(id_##x);
#define ARG_OBJECT_FROM_ID(x) ARG_OBJID(id_##x); \
                      Object* x = getObject(id_##x);

#define ARG_ITEM_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      Item* x = getItem(id_##x);
#define ARG_ITEM_FROM_ID(x)   ARG_OBJID(id_##x); \
                      Item* x = getItem(id_##x);

#define ARG_CONTAINER_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      Container* x = getContainer(id_##x);
#define ARG_CONTAINER_FROM_ID(x) ARG_OBJID(id_##x); \
                      Container* x = getContainer(id_##x);

#define ARG_ACTOR_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      Actor* x = getActor(id_##x);
#define ARG_ACTOR_FROM_ID(x)  ARG_OBJID(id_##x); \
                      Actor* x = getActor(id_##x);

#define ARG_EGG_FROM_PTR(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      Egg* x = p_dynamic_cast<Egg*>(getObject(id_##x));
#define ARG_EGG_FROM_ID(x)    ARG_OBJID(id_##x); \
                      Egg* x = p_dynamic_cast<Egg*>(getObject(id_##x));

#define ARG_STRING(x) ARG_UC_PTR(ucptr_##x); \
                      uint16 id_##x = UCMachine::ptrToObject(ucptr_##x); \
                      std::string x = UCMachine::get_instance()->getString(id_##x);

#define ARG_LIST(x)   ARG_UINT16(id_##x); \
                      UCList* x = UCMachine::get_instance()->getList(id_##x);

#define ARG_WORLDPOINT(x) ARG_UC_PTR(ucptr_##x); \
					  WorldPoint x; \
					  UCMachine::get_instance()->dereferencePointer(ucptr_##x, x.buf, 5);

#define ARG_NULL8()  args+=1;
#define ARG_NULL16() args+=2;
#define ARG_NULL32() args+=4;

#endif
