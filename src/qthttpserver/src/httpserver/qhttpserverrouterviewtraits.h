/****************************************************************************
**
** Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtHttpServer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QHTTPSERVERROUTERVIEWTRAITS_H
#define QHTTPSERVERROUTERVIEWTRAITS_H

#include <QtHttpServer/qhttpserverviewtraits_impl.h>

QT_BEGIN_NAMESPACE

class QHttpServerRequest;
class QHttpServerResponder;

namespace QtPrivate {

template<typename ViewHandler, bool DisableStaticAssert>
struct RouterViewTraitsHelper : ViewTraits<ViewHandler, DisableStaticAssert> {
    using VTraits = ViewTraits<ViewHandler, DisableStaticAssert>;
    using FunctionTraits = typename VTraits::FTraits;

    template<int I>
    struct ArgumentChecker : FunctionTraits::template Arg<I> {
        using IsRequest = typename VTraits::template Special<I, const QHttpServerRequest &>;
        static_assert(IsRequest::AssertCondition,
                      "ViewHandler arguments error: "
                      "QHttpServerRequest can only be passed as a const reference");

        using IsResponder = typename VTraits::template Special<I, QHttpServerResponder &&>;
        static_assert(IsResponder::AssertCondition,
                      "ViewHandler arguments error: "
                      "QHttpServerResponder can only be passed as a universal reference");

        using IsSpecial = CheckAny<IsRequest, IsResponder>;

        struct IsSimple {
            static constexpr bool Value = !IsSpecial::Value &&
                                           I < FunctionTraits::ArgumentCount &&
                                           FunctionTraits::ArgumentIndexMax != -1;
            static constexpr bool Valid = FunctionTraits::template Arg<I>::Defined;

            static constexpr bool StaticAssert =
                DisableStaticAssert || !Value || Valid;


            static_assert(StaticAssert,
                          "ViewHandler arguments error: "
                          "Type is not registered, please use the Q_DECLARE_METATYPE macro "
                          "to make it known to Qt's meta-object system");
        };

        using CheckOk = CheckAny<IsSimple, IsSpecial>;

        static constexpr bool Valid = CheckOk::Valid;
        static constexpr bool StaticAssert = CheckOk::StaticAssert;
    };


    struct Arguments {
        template<int ... I>
        struct ArgumentsReturn {
            template<int Idx>
            using Arg = ArgumentChecker<Idx>;

            template<int Idx>
            static constexpr int metaTypeId() noexcept
            {
                using Type = typename FunctionTraits::template Arg<Idx>::CleanType;

                return qMetaTypeId<
                    typename std::conditional<
                        QMetaTypeId2<Type>::Defined,
                        Type,
                        void>::type>();
            }

            static constexpr std::size_t Count = FunctionTraits::ArgumentCount;
            static constexpr std::size_t CapturableCount =
                StaticMath::Sum::eval(
                    static_cast<std::size_t>(FunctionTraits::template Arg<I>::Defined)...);
            static constexpr std::size_t PlaceholdersCount = Count - CapturableCount;

            static constexpr bool Valid = StaticMath::And::eval(Arg<I>::Valid...);
            static constexpr bool StaticAssert =
                StaticMath::And::eval(Arg<I>::StaticAssert...);

            using Indexes = typename QtPrivate::IndexesList<I...>;

            using CapturableIndexes =
                typename QtPrivate::Indexes<CapturableCount>::Value;

            using PlaceholdersIndexes =
                typename QtPrivate::Indexes<PlaceholdersCount>::Value;

            using Last = Arg<FunctionTraits::ArgumentIndexMax>;
        };

        template<int ... I>
        static constexpr ArgumentsReturn<I...> eval(QtPrivate::IndexesList<I...>) noexcept
        {
            return ArgumentsReturn<I...>{};
        }
    };

    template<int CaptureOffset>
    struct BindType {
        template<typename ... Args>
        struct FunctionWrapper {
            using Type = std::function<typename FunctionTraits::ReturnType (Args...)>;
        };

        template<int Id>
        using OffsetArg = typename FunctionTraits::template Arg<CaptureOffset + Id>::Type;

        template<int ... Idx>
        static constexpr typename FunctionWrapper<OffsetArg<Idx>...>::Type
                eval(QtPrivate::IndexesList<Idx...>) noexcept;
    };
};


} // namespace QtPrivate

template <typename ViewHandler, bool DisableStaticAssert = false>
struct QHttpServerRouterViewTraits
{
    using Helpers = typename QtPrivate::RouterViewTraitsHelper<ViewHandler, DisableStaticAssert>;
    using ReturnType = typename Helpers::FunctionTraits::ReturnType;
    using Arguments = decltype(Helpers::Arguments::eval(typename Helpers::ArgumentIndexes{}));
    using BindableType = decltype(
            Helpers::template BindType<Arguments::CapturableCount>::eval(
                typename Arguments::PlaceholdersIndexes{}));
};


QT_END_NAMESPACE

#endif  // QHTTPSERVERROUTERVIEWTRAITS_H
