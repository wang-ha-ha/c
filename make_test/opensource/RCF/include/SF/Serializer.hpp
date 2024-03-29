
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

/// @file

#ifndef INCLUDE_SF_SERIALIZER_HPP
#define INCLUDE_SF_SERIALIZER_HPP

#include <RCF/Exception.hpp>
#include <RCF/Export.hpp>
#include <RCF/MemStream.hpp>
#include <RCF/TypeTraits.hpp>

#include <SF/Archive.hpp>
#include <SF/I_Stream.hpp>
#include <SF/SerializeFundamental.hpp>
#include <SF/SfNew.hpp>
#include <RCF/Tools.hpp>

namespace SF {

    // Generic serializer, subclassed by all other serializers.

    class RCF_EXPORT SerializerBase : Noncopyable
    {
    private:
        void                invokeRead(Archive &ar);
        void                invokeWrite(Archive &ar);

        // Following are overridden to provide type-specific operations.
        virtual std::string getTypeName() = 0;
        virtual void        newObject(Archive &ar) = 0;
        virtual bool        isDerived() = 0;
        virtual std::string getDerivedTypeName() = 0;
        virtual void        getSerializerPolymorphic(const std::string &derivedTypeName) = 0;
        virtual void        invokeSerializerPolymorphic(SF::Archive &) = 0;
        virtual void        serializeContents(Archive &ar) = 0;
        virtual void        addToInputContext(IStream *, const UInt32 &) = 0;
        virtual bool        queryInputContext(IStream *, const UInt32 &) = 0;
        virtual void        addToOutputContext(OStream *, UInt32 &) = 0;
        virtual bool        queryOutputContext(OStream *, UInt32 &) = 0;
        virtual void        setFromId() = 0;
        virtual void        setToNull() = 0;
        virtual bool        isNull() = 0;
        virtual bool        isNonAtomic() = 0;

    public:
                            SerializerBase();
        virtual             ~SerializerBase();
        void                invoke(Archive &ar);
    };    

    //---------------------------------------------------------------------
    // Type-specific serializers

    // These pragmas concern Serializer<T>::newObject, but needs to be up here, probably because Serializer<T> is a template
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4675 )  // warning C4675: resolved overload was found by argument-dependent lookup
#pragma warning( disable : 4702 )  // warning C4702: unreachable code
#endif

    class I_SerializerPolymorphic;

    template<typename T>
    class Serializer : public SerializerBase
    {
    public:
        Serializer(T ** ppt);

    private:
        typedef ObjectId IdT;
        T **                        ppt;
        I_SerializerPolymorphic *   pf;
        IdT                         id;

        std::string         getTypeName();
        void                newObject(Archive &ar);
        bool                isDerived();
        std::string         getDerivedTypeName();
        void                getSerializerPolymorphic(const std::string &derivedTypeName);
        void                invokeSerializerPolymorphic(SF::Archive &ar);
        void                serializeContents(Archive &ar);
        void                addToInputContext(SF::IStream *stream, const UInt32 &nid);
        bool                queryInputContext(SF::IStream *stream, const UInt32 &nid);
        void                addToOutputContext(SF::OStream *stream, UInt32 &nid);
        bool                queryOutputContext(SF::OStream *stream, UInt32 &nid);
        void                setFromId();
        void                setToNull();
        bool                isNull();
        bool                isNonAtomic();
    };

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    template<typename PPT>
    struct GetIndirection
    {
        typedef typename RCF::RemovePointer<PPT>::type    PT;
        typedef typename RCF::IsPointer<PPT>::type        is_single;
        typedef typename RCF::IsPointer<PT>::type         is_double;

        typedef
        typename RCF::If<
            is_double,
            RCF::Int<2>,
            typename RCF::If<
                is_single,
                RCF::Int<1>,
                RCF::Int<0>
            >::type
        >::type Level;

        typedef
        typename RCF::RemoveCv<
            typename RCF::RemovePointer<
                typename RCF::RemoveCv<
                    typename RCF::RemovePointer<
                        typename RCF::RemoveCv<PPT>::type
                    >::type
                >::type
            >::type
        >::type Base;
    };

    template<typename T>
    inline void invokeCustomSerializer(
        T **ppt, 
        Archive &ar, 
        int)
    {
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        Serializer<T>(ppt).invoke(ar);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        RCF::Int<0> *,
        const U &u, 
        Archive &ar)
    {
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        T *pt = const_cast<T *>(&u);
        invokeCustomSerializer( (T **) (&pt), ar, 0);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        RCF::Int<1> *,
        const U &u, 
        Archive &ar)
    {
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        invokeCustomSerializer( (T **) (&u), ar, 0);
    }

    template<typename U, typename T>
    inline void invokeSerializer(
        U *, 
        T *, 
        RCF::Int<2> *,
        const U &u, 
        Archive &ar)
    {
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        invokeCustomSerializer( (T**) (u), ar, 0);
    }

    template<typename U>
    inline void invokeSerializer(U u, Archive &ar)
    {
        typedef typename GetIndirection<U>::Level Level;
        typedef typename GetIndirection<U>::Base T;
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        invokeSerializer( (U *) 0, (T *) 0, (Level *) 0, u, ar);
    }

    template<typename U>
    inline void invokePtrSerializer(U u, Archive &ar)
    {
        typedef typename GetIndirection<U>::Level Level;
        const int levelOfIndirection = Level::value;
        RCF_ASSERT( levelOfIndirection == 1 || levelOfIndirection == 2);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6326) // warning C6326: Potential comparison of a constant with another constant.
#endif

        ar.setFlag( SF::Archive::POINTER, levelOfIndirection == 2 );

#ifdef _MSC_VER
#pragma warning(pop)
#endif

        invokeSerializer(u, ar);
    }

    // Forward declaration.
    template<typename T> 
    Archive & operator&(
        Archive &                           archive, 
        const T &                           t);

    template<typename T>
    inline void serializeEnum(SF::Archive &ar, T &t)
    {
        int runtimeVersion = ar.getRuntimeVersion();
        if (runtimeVersion >= 2)
        {
            ar & SF::Archive::Flag(SF::Archive::NO_BEGIN_END);
        }

        if (ar.isRead())
        {
            std::int32_t n = 0;
            ar & n;
            t = T(n);
        }
        else /* if (ar.isWrite())) */
        {
            std::int32_t n = t;
            ar & n;
        }
    }

    template<typename T>
    inline void serializeInternal(Archive &archive, T &t)
    {
        // A compiler error here indicates that the class T has not implemented
        // an internal SF serialization function. Here are a few situations in
        // which this can happen:

        // *  No serialization function was provided at all, in which case one
        //    needs to be written (either internal or external).
        // 
        // *  The intention was to provide an external SF serialization function, 
        //    but the external serialization function definition is incorrect and 
        //    hence not visible to the framework.
               
        // *  The class is a Protocol Buffers generated class, and the intention 
        //    was to serialize it as such, in which case RCF_FEATURE_PROTOBUF=1
        //    needs to be defined.

        t.serialize(archive);
    }

    template<typename T>
    inline void serializeFundamentalOrNot(
        Archive &                           archive, 
        T &                                 t, 
        RCF::TrueType *)
    {
        serializeFundamental(archive, t);
    }

    template<typename T>
    inline void serializeFundamentalOrNot(
        Archive &                           archive, 
        T &                                 t, 
        RCF::FalseType *)
    {
        serializeInternal(archive, t);
    }

    template<typename T>
    inline void serializeEnumOrNot(
        Archive &                           archive, 
        T &                                 t, 
        RCF::TrueType *)
    {
        serializeEnum(archive, t);
    }

    template<typename T>
    inline void serializeEnumOrNot(
        Archive &                           archive, 
        T &                                 t, 
        RCF::FalseType *)
    {
        typedef typename RCF::IsFundamental<T>::type type;
        serializeFundamentalOrNot(archive, t, (type *) NULL);
    }

    template<typename T>
    inline void serialize(
        Archive &                           archive, 
        T &                                 t)
    {
        typedef typename std::is_enum<T>::type type;
        serializeEnumOrNot(archive, t, (type *) NULL);
    }

    template<typename T>
    inline void serialize_vc6(
        Archive &                           archive, 
        T &                                 t,
        const unsigned int)
    {
        serialize(archive, t);
    }

    template<typename T>
    inline void preserialize(
        Archive &                           archive, 
        T *&                                pt,
        const unsigned int)
    {
        static_assert(!RCF::IsPointer<T>::value, "Incorrect serialization code.");
        typedef typename RCF::RemoveCv<T>::type U;
        serialize_vc6(archive, (U &) *pt, static_cast<const unsigned int>(0) );
    }

    template<typename T> 
    Archive & operator&(
        Archive &                           archive, 
        const T &                           t)
    {
        invokePtrSerializer(&t, archive);
        return archive;
    }

    template<typename T, typename U>
    inline void serializeAs(Archive & ar, T &t)
    {
        if (ar.isWrite())
        {
            U u = static_cast<U>(t);
            ar & u;
        }
        else
        {
            U u;
            ar & u;
            t = static_cast<T>(u);
        }
    }

/// Instructs RCF to serialize enum class EnumType, as a BaseType object.
#define SF_SERIALIZE_ENUM_CLASS(EnumType, BaseType)             \
    void serialize(SF::Archive & ar, EnumType & e)              \
    {                                                           \
        SF::serializeAs<EnumType, BaseType>(ar, e);             \
    }

}

#include <SF/Registry.hpp>
#include <SF/SerializePolymorphic.hpp>
#include <SF/Stream.hpp>

namespace SF {

    template<typename T>
    Serializer<T>::Serializer(T **ppt) :
        ppt(ppt),
        pf(),
        id()
    {}

    template<typename T>
    std::string Serializer<T>::getTypeName()
    {
        return SF::Registry::getSingleton().getTypeName( (T *) 0);
    }

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif

    template<typename T>
    void Serializer<T>::newObject(Archive &ar)
    {
        *ppt = sfNew((T *) NULL, (T **) NULL, ar);
    }

#ifdef _MSC_VER
#pragma warning( pop )
#endif

    template<typename T>
    bool Serializer<T>::isDerived()
    {
        static const bool isFundamental = RCF::IsFundamental<T>::value;
        if (!isFundamental && *ppt && typeid(T) != typeid(**ppt))
        {
            if (!SF::Registry::getSingleton().isTypeRegistered( typeid(**ppt)))
            {
                RCF::Exception e(RCF::RcfError_SfTypeRegistration, typeid(**ppt).name());
                RCF_THROW(e);
            }
            return true;
        }
        return false;
    }

    template<typename T>
    std::string Serializer<T>::getDerivedTypeName()
    {
        return SF::Registry::getSingleton().getTypeName( typeid(**ppt) );
    }

    template<typename T>
    void Serializer<T>::getSerializerPolymorphic(
        const std::string &derivedTypeName)
    {
        pf = & SF::Registry::getSingleton().getSerializerPolymorphic( 
            (T *) 0, 
            derivedTypeName);
    }

    template<typename T>
    void Serializer<T>::invokeSerializerPolymorphic(SF::Archive &ar)
    {
        RCF_ASSERT(pf);
        void **ppvb = (void **) (ppt); // not even reinterpret_cast wants to touch this
        pf->invoke(ppvb, ar);
    }

    template<typename T>
    void Serializer<T>::serializeContents(Archive &ar)
    {
        preserialize(ar, *ppt, 0);
    }

    template<typename T>
    void Serializer<T>::addToInputContext(SF::IStream *stream, const UInt32 &nid)
    {
        ContextRead &ctx = stream->getTrackingContext();
        ctx.add(nid, IdT( (void *) (*ppt), &typeid(T)));
    }

    template<typename T>
    bool Serializer<T>::queryInputContext(SF::IStream *stream, const UInt32 &nid)
    {
        ContextRead &ctx = stream->getTrackingContext();
        return ctx.query(nid, id);
    }

    template<typename T>
    void Serializer<T>::addToOutputContext(SF::OStream *stream, UInt32 &nid)
    {
        ContextWrite &ctx = stream->getTrackingContext();
        ctx.add( IdT( (void *) *ppt, &typeid(T)), nid);
    }

    template<typename T>
    bool Serializer<T>::queryOutputContext(SF::OStream *stream, UInt32 &nid)
    {
        ContextWrite &ctx = stream->getTrackingContext();
        return ctx.query( IdT( (void *) *ppt, &typeid(T)), nid);
    }

    template<typename T>
    void Serializer<T>::setFromId()
    {
        *ppt = reinterpret_cast<T *>(id.first);
    }

    template<typename T>
    void Serializer<T>::setToNull()
    {
        *ppt = NULL;
    }

    template<typename T>
    bool Serializer<T>::isNull()
    {
        return *ppt == NULL;
    }

    template<typename T>
    bool Serializer<T>::isNonAtomic()
    {
        bool isFundamental = RCF::IsFundamental<T>::value;
        return !isFundamental;
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZER_HPP
