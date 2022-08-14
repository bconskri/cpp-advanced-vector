#include "vector.h"

#include <stdexcept>

namespace {

    // Магическое число, используемое для отслеживания живости объекта
    inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj7 {
        TestObj7() = default;
        TestObj7(const TestObj7& other) = default;
        TestObj7& operator=(const TestObj7& other) = default;
        TestObj7(TestObj7&& other) = default;
        TestObj7& operator=(TestObj7&& other) = default;
        ~TestObj7() {
            cookie = 0;
        }
        [[nodiscard]] bool IsAlive() const noexcept {
            return cookie == DEFAULT_COOKIE;
        }
        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj7 {
        Obj7() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj7(int id)
                : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj7(const Obj7& other)
                : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj7(Obj7&& other) noexcept
                : id(other.id)  //
        {
            ++num_moved;
        }

        Obj7& operator=(const Obj7& other) = default;
        Obj7& operator=(Obj7&& other) = default;

        ~Obj7() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj7ectCount() {
            return num_default_constructed + num_copied + num_moved + num_constructed_with_id
                   - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
            num_constructed_with_id = 0;
        }

        bool throw_on_copy = false;
        int id = 0;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

}  // namespace

void Test9_1() {
    Obj7::ResetCounters();
    const size_t SIZE = 100500;
    const size_t INDEX = 10;
    const int MAGIC = 42;
    {
        Vector<int> v;
        assert(v.Capacity() == 0);
        assert(v.Size() == 0);

        v.Reserve(SIZE);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == 0);
    }
    {
        Vector<int> v(SIZE);
        const auto& cv(v);
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(v[0] == 0);
        assert(&v[0] == &cv[0]);
        v[INDEX] = MAGIC;
        assert(v[INDEX] == MAGIC);
        assert(&v[100] - &v[0] == 100);

        v.Reserve(SIZE * 2);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE * 2);
        assert(v[INDEX] == MAGIC);
    }
    {
        Vector<int> v(SIZE);
        v[INDEX] = MAGIC;
        const auto v_copy(v);
        assert(&v[INDEX] != &v_copy[INDEX]);
        assert(v[INDEX] == v_copy[INDEX]);
    }
    {
        Vector<Obj7> v;
        v.Reserve(SIZE);
        assert(Obj7::GetAliveObj7ectCount() == 0);
    }
    {
        Vector<Obj7> v(SIZE);
        assert(Obj7::GetAliveObj7ectCount() == SIZE);
        const int old_copy_count = Obj7::num_copied;
        const int old_move_count = Obj7::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj7::GetAliveObj7ectCount() == SIZE);
        assert(Obj7::num_copied == old_copy_count);
        assert(Obj7::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj7::GetAliveObj7ectCount() == 0);
}

void Test9_2() {
    const size_t SIZE = 100;
    Obj7::ResetCounters();
    {
        Obj7::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj7> v(SIZE);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj7::num_default_constructed == SIZE / 2 - 1);
        assert(Obj7::GetAliveObj7ectCount() == 0);
    }
    Obj7::ResetCounters();
    {
        Vector<Obj7> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj7> v_copy(v);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj7::num_copied == SIZE / 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj7::GetAliveObj7ectCount() == SIZE);
    }
    Obj7::ResetCounters();
    {
        Vector<Obj7> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj7::GetAliveObj7ectCount() == SIZE);
    }
}

void Test9_3() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj7::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj7::GetAliveObj7ectCount() == 0);
    }
    {
        Obj7::ResetCounters();
        {
            Vector<Obj7> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj7::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj7> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj7::GetAliveObj7ectCount() == 0);

        assert(Obj7::num_moved == 0);
        assert(Obj7::num_copied == 0);
        assert(Obj7::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj7::ResetCounters();
        Vector<Obj7> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj7> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj7::GetAliveObj7ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj7::ResetCounters();
        Vector<Obj7> v(MEDIUM_SIZE);
        {
            Vector<Obj7> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj7::GetAliveObj7ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj7::GetAliveObj7ectCount() == LARGE_SIZE);
    }
    assert(Obj7::GetAliveObj7ectCount() == 0);
    {
        Obj7::ResetCounters();
        Vector<Obj7> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj7> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj7::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj7::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}

void Test9_4() {
    const size_t ID = 42;
    const size_t SIZE = 100'500;
    {
        Obj7::ResetCounters();
        Vector<Obj7> v;
        v.Resize(SIZE);
        assert(v.Size() == SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj7::num_default_constructed == SIZE);
    }
    assert(Obj7::GetAliveObj7ectCount() == 0);

    {
        const size_t NEW_SIZE = 10'000;
        Obj7::ResetCounters();
        Vector<Obj7> v(SIZE);
        v.Resize(NEW_SIZE);
        assert(v.Size() == NEW_SIZE);
        assert(v.Capacity() == SIZE);
        assert(Obj7::num_destroyed == SIZE - NEW_SIZE);
    }
    assert(Obj7::GetAliveObj7ectCount() == 0);
    {
        Obj7::ResetCounters();
        Vector<Obj7> v(SIZE);
        Obj7 o{ID};
        v.PushBack(o);
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj7::num_default_constructed == SIZE);
        assert(Obj7::num_copied == 1);
        assert(Obj7::num_constructed_with_id == 1);
        assert(Obj7::num_moved == SIZE);
    }
    assert(Obj7::GetAliveObj7ectCount() == 0);
    {
        Obj7::ResetCounters();
        Vector<Obj7> v(SIZE);
        v.PushBack(Obj7{ID});
        assert(v.Size() == SIZE + 1);
        assert(v.Capacity() == SIZE * 2);
        assert(v[SIZE].id == ID);
        assert(Obj7::num_default_constructed == SIZE);
        assert(Obj7::num_copied == 0);
        assert(Obj7::num_constructed_with_id == 1);
        assert(Obj7::num_moved == SIZE + 1);
    }
    {
        Obj7::ResetCounters();
        Vector<Obj7> v;
        v.PushBack(Obj7{ID});
        v.PopBack();
        assert(v.Size() == 0);
        assert(v.Capacity() == 1);
        assert(Obj7::GetAliveObj7ectCount() == 0);
    }

    {
        Vector<TestObj7> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(v[0]);
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
    {
        Vector<TestObj7> v(1);
        assert(v.Size() == v.Capacity());
        // Операция PushBack для перемещения существующего элемента вектора должна быть безопасна
        // даже при реаллокации памяти
        v.PushBack(std::move(v[0]));
        assert(v[0].IsAlive());
        assert(v[1].IsAlive());
    }
}