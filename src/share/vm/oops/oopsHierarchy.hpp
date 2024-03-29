/*
 * Copyright (c) 1997, 2011, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_OOPS_OOPSHIERARCHY_HPP
#define SHARE_VM_OOPS_OOPSHIERARCHY_HPP

#include "runtime/globals.hpp"
#include "utilities/globalDefinitions.hpp"

// OBJECT hierarchy
// This hierarchy is a representation hierarchy, i.e. if A is a superclass
// of B, A's representation is a prefix of B's representation.

typedef juint narrowOop; // Offset instead of address for an oop within a java object
typedef class klassOopDesc* wideKlassOop; // to keep SA happy and unhandled oop
                                          // detector happy.
typedef void* OopOrNarrowOopStar;

// oop代表一个实例对象。
// 普通对象指针oop定义，统一去掉了Desc后缀，简化了名称。
#ifndef CHECK_UNHANDLED_OOPS

typedef class oopDesc*                            oop; // 所有oop的顶级父类
typedef class   instanceOopDesc*            instanceOop; // 表示Java类实例
typedef class   methodOopDesc*                    methodOop; // 表示Java方法
typedef class   constMethodOopDesc*            constMethodOop; // 表示Java方法中的只读信息（其实就是字节码指令）
typedef class   methodDataOopDesc*            methodDataOop; // 表示性能统计的相关数据
typedef class   arrayOopDesc*                    arrayOop; // 数组对象
typedef class     objArrayOopDesc*            objArrayOop; // 表示引用类型数组对象
typedef class     typeArrayOopDesc*            typeArrayOop; // 表示基本类型数组对象
typedef class   constantPoolOopDesc*            constantPoolOop; // 表示Java字节码文件中的常量池
typedef class   constantPoolCacheOopDesc*   constantPoolCacheOop; // 与constantPoolOop相伴生，是后者的缓存对象
typedef class   klassOopDesc*                    klassOop; // 指向JVM内部的klass实例的对象
typedef class   markOopDesc*                    markOop; // oop的标记对象
typedef class   compiledICHolderOopDesc*    compiledICHolderOop;

#else // When CHECK_UNHANDLED_OOPS is defined


// When CHECK_UNHANDLED_OOPS is defined, an "oop" is a class with a
// carefully chosen set of constructors and conversion operators to go
// to and from the underlying oopDesc pointer type.
//
// Because oop and its subclasses <type>Oop are class types, arbitrary
// conversions are not accepted by the compiler, and you may get a message
// about overloading ambiguity (between long and int is common when converting
// from a constant in 64 bit mode), or unable to convert from type to 'oop'.
// Applying a cast to one of these conversion operators first will get to the
// underlying oopDesc* type if appropriate.
// Converting NULL to oop to Handle implicit is no longer accepted by the
// compiler because there are too many steps in the conversion.  Use Handle()
// instead, which generates less code anyway.

class Thread;
typedef class   markOopDesc*                markOop;
class PromotedObject;


class oop {
  oopDesc* _o;

  void register_oop();
  void unregister_oop();

  // friend class markOop;
public:
  void set_obj(const void* p)         {
    raw_set_obj(p);
    if (CheckUnhandledOops) register_oop();
  }
  void raw_set_obj(const void* p)     { _o = (oopDesc*)p; }

  oop()                               { set_obj(NULL); }
  oop(const volatile oop& o)          { set_obj(o.obj()); }
  oop(const void* p)                  { set_obj(p); }
  oop(intptr_t i)                     { set_obj((void *)i); }
#ifdef _LP64
  oop(int i)                          { set_obj((void *)i); }
#endif
  ~oop()                              {
    if (CheckUnhandledOops) unregister_oop();
  }

  oopDesc* obj()  const volatile      { return _o; }

  // General access
  oopDesc*  operator->() const        { return obj(); }
  bool operator==(const oop o) const  { return obj() == o.obj(); }
  bool operator==(void *p) const      { return obj() == p; }
  bool operator!=(const oop o) const  { return obj() != o.obj(); }
  bool operator!=(void *p) const      { return obj() != p; }
  bool operator==(intptr_t p) const   { return obj() == (oopDesc*)p; }
  bool operator!=(intptr_t p) const   { return obj() != (oopDesc*)p; }

  bool operator<(oop o) const         { return obj() < o.obj(); }
  bool operator>(oop o) const         { return obj() > o.obj(); }
  bool operator<=(oop o) const        { return obj() <= o.obj(); }
  bool operator>=(oop o) const        { return obj() >= o.obj(); }
  bool operator!() const              { return !obj(); }

  // Cast
  operator void* () const             { return (void *)obj(); }
  operator HeapWord* () const         { return (HeapWord*)obj(); }
  operator oopDesc* () const          { return obj(); }
  operator intptr_t* () const         { return (intptr_t*)obj(); }
  operator PromotedObject* () const   { return (PromotedObject*)obj(); }
  operator markOop () const           { return markOop(obj()); }

  operator address   () const         { return (address)obj(); }
  operator intptr_t () const          { return (intptr_t)obj(); }

  // from javaCalls.cpp
  operator jobject () const           { return (jobject)obj(); }
  // from javaClasses.cpp
  operator JavaThread* () const       { return (JavaThread*)obj(); }

#ifndef _LP64
  // from jvm.cpp
  operator jlong* () const            { return (jlong*)obj(); }
#endif

  // from parNewGeneration and other things that want to get to the end of
  // an oop for stuff (like constMethodKlass.cpp, objArrayKlass.cpp)
  operator oop* () const              { return (oop *)obj(); }
};

#define DEF_OOP(type)                                                      \
   class type##OopDesc;                                                    \
   class type##Oop : public oop {                                          \
     public:                                                               \
       type##Oop() : oop() {}                                              \
       type##Oop(const volatile oop& o) : oop(o) {}                        \
       type##Oop(const void* p) : oop(p) {}                                \
       operator type##OopDesc* () const { return (type##OopDesc*)obj(); }  \
       type##OopDesc* operator->() const {                                 \
            return (type##OopDesc*)obj();                                  \
       }                                                                   \
   };                                                                      \

DEF_OOP(instance);
DEF_OOP(method);
DEF_OOP(methodData);
DEF_OOP(array);
DEF_OOP(constMethod);
DEF_OOP(constantPool);
DEF_OOP(constantPoolCache);
DEF_OOP(objArray);
DEF_OOP(typeArray);
DEF_OOP(klass);
DEF_OOP(compiledICHolder);

#endif // CHECK_UNHANDLED_OOPS

// The klass hierarchy is separate from the oop hierarchy.
// klass层次结构与oop层次结构是分开的。
// Klass表示类的class信息，即Java类元数据在方法区中的表示。

class Klass; // klass家族的基类
class   instanceKlass; // 虚拟机层面与Java类对等的数据结构。假设有Java类ClassA，那么instanceKlass就是ClassA这个类类型结构的对等体
class     instanceMirrorKlass; // 描述java.lang.Class的实例
class     instanceRefKlass; // 描述java.lang.ref.Reference的子类
class   methodKlass; // 表示Java类的方法
class   constMethodKlass; // 描述Java类方法所对应的字节码指令信息的固有属性
class   methodDataKlass;
class   klassKlass; // klass链路的末端
class     instanceKlassKlass;
class     arrayKlassKlass;
class       objArrayKlassKlass;
class       typeArrayKlassKlass;
class   arrayKlass; // 描述Java数组的信息，是个抽象基类
class     objArrayKlass; // 描述Java中引用类型数组的数据结构
class     typeArrayKlass; // 描述Java中基本类型数组的数据结构
class   constantPoolKlass; // 描述Java字节码文件中的常量池的数据结构
class   constantPoolCacheKlass;
class   compiledICHolderKlass;

#endif // SHARE_VM_OOPS_OOPSHIERARCHY_HPP
