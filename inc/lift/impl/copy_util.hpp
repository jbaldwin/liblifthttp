#pragma once

#include <memory>
#include <optional>

namespace lift::impl
{
/**
 * This class is a work-around in that std::function does not support move only types.
 * std::promise<T> is move only, and during a times up event it needs to be 'moved' to the client
 * via a custom copy/move.  This type is pretty ugly but its only used for this one use-case.
 *
 * Other viable solutions could be implementing a custom function type that works for move only types
 * or taking a performance hit at runtime and wrapping the std::promise in an std::shared_ptr.
 */
template<typename T>
struct copy_but_actually_move
{
    explicit copy_but_actually_move(T t) : m_object(std::move(t)) {}
    ~copy_but_actually_move() = default;

    copy_but_actually_move(const copy_but_actually_move<T>& other) : m_object(std::move(other.m_object)) {}
    copy_but_actually_move(copy_but_actually_move<T>&& other) : m_object(std::move(other.m_object)) {}

    auto operator=(const copy_but_actually_move<T>& other) -> copy_but_actually_move<T>&
    {
        if (std::addressof(other) != this)
        {
            m_object = std::move(other.m_object);
        }
        return *this;
    }

    auto operator=(copy_but_actually_move<T>&& other) -> copy_but_actually_move<T>&
    {
        if (std::addressof(other) != this)
        {
            m_object = std::move(other.m_object);
        }
        return *this;
    }

    mutable std::optional<T> m_object{std::nullopt};
};

} // namespace lift::impl
