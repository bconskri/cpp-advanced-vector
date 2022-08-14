#include "vector.h"

#include <stdexcept>

namespace {

    struct Obj1 {
        Obj1() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        Obj1(const Obj1& other) {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj1(Obj1&& /*other*/) noexcept {
            ++num_moved;
        }

        Obj1& operator=(const Obj1& other) = default;
        Obj1& operator=(Obj1&& other) = default;

        ~Obj1() {
            ++num_destroyed;
        }

        static int GetAliveObj1ectCount() {
            return num_default_constructed + num_copied + num_moved - num_destroyed;
        }

        static void ResetCounters() {
            default_construction_throw_countdown = 0;
            num_default_constructed = 0;
            num_copied = 0;
            num_moved = 0;
            num_destroyed = 0;
        }

        bool throw_on_copy = false;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
    };

    struct Obj8 {
        Obj8() {
            if (default_construction_throw_countdown > 0) {
                if (--default_construction_throw_countdown == 0) {
                    throw std::runtime_error("Oops");
                }
            }
            ++num_default_constructed;
        }

        explicit Obj8(int id)
                : id(id)  //
        {
            ++num_constructed_with_id;
        }

        Obj8(const Obj8& other)
                : id(other.id)  //
        {
            if (other.throw_on_copy) {
                throw std::runtime_error("Oops");
            }
            ++num_copied;
        }

        Obj8(Obj8&& other) noexcept
                : id(other.id)  //
        {
            ++num_moved;
        }

        Obj8& operator=(const Obj8& other) = default;
        Obj8& operator=(Obj8&& other) = default;

        ~Obj8() {
            ++num_destroyed;
            id = 0;
        }

        static int GetAliveObj8ectCount() {
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

void Test1() {
    Obj1::ResetCounters();
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
        Vector<Obj1> v;
        v.Reserve(SIZE);
        assert(Obj1::GetAliveObj1ectCount() == 0);
    }
    {
        Vector<Obj1> v(SIZE);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
        v.Reserve(SIZE * 2);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
    }
    assert(Obj1::GetAliveObj1ectCount() == 0);
}

void Test2() {
    const size_t SIZE = 100;
    Obj1::ResetCounters();
    {
        Obj1::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj1> v(SIZE);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::num_default_constructed == SIZE / 2 - 1);
        assert(Obj1::GetAliveObj1ectCount() == 0);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj1> v_copy(v);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj1::num_copied == SIZE / 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj1::num_copied == SIZE - 1);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE);
        assert(v.Size() == SIZE);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
    }
}

void Test3() {
    Obj1::ResetCounters();
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
        Vector<Obj1> v;
        v.Reserve(SIZE);
        assert(Obj1::GetAliveObj1ectCount() == 0);
    }
    {
        Vector<Obj1> v(SIZE);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
        const int old_copy_count = Obj1::num_copied;
        const int old_move_count = Obj1::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
        assert(Obj1::num_copied == old_copy_count);
        assert(Obj1::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj1::GetAliveObj1ectCount() == 0);
}

void Test4() {
    const size_t SIZE = 100;
    Obj1::ResetCounters();
    {
        Obj1::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj1> v(SIZE);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::num_default_constructed == SIZE / 2 - 1);
        assert(Obj1::GetAliveObj1ectCount() == 0);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj1> v_copy(v);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj1::num_copied == SIZE / 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
    }
    Obj1::ResetCounters();
    {
        Vector<Obj1> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj1::GetAliveObj1ectCount() == SIZE);
    }
}

void Test6() {
    Obj8::ResetCounters();
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
        Vector<Obj8> v;
        v.Reserve(SIZE);
        assert(Obj8::GetAliveObj8ectCount() == 0);
    }
    {
        Vector<Obj8> v(SIZE);
        assert(Obj8::GetAliveObj8ectCount() == SIZE);
        const int old_copy_count = Obj8::num_copied;
        const int old_move_count = Obj8::num_moved;
        v.Reserve(SIZE * 2);
        assert(Obj8::GetAliveObj8ectCount() == SIZE);
        assert(Obj8::num_copied == old_copy_count);
        assert(Obj8::num_moved == old_move_count + static_cast<int>(SIZE));
    }
    assert(Obj8::GetAliveObj8ectCount() == 0);
}

void Test7() {
    const size_t SIZE = 100;
    Obj8::ResetCounters();
    {
        Obj8::default_construction_throw_countdown = SIZE / 2;
        try {
            Vector<Obj8> v(SIZE);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj8::num_default_constructed == SIZE / 2 - 1);
        assert(Obj8::GetAliveObj8ectCount() == 0);
    }
    Obj8::ResetCounters();
    {
        Vector<Obj8> v(SIZE);
        try {
            v[SIZE / 2].throw_on_copy = true;
            Vector<Obj8> v_copy(v);
            assert(false && "Exception is expected");
        } catch (const std::runtime_error&) {
            assert(Obj8::num_copied == SIZE / 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(Obj8::GetAliveObj8ectCount() == SIZE);
    }
    Obj8::ResetCounters();
    {
        Vector<Obj8> v(SIZE);
        try {
            v[SIZE - 1].throw_on_copy = true;
            v.Reserve(SIZE * 2);
        } catch (...) {
            // Unexpected error
            assert(false && "Unexpected exception");
        }
        assert(v.Capacity() == SIZE * 2);
        assert(v.Size() == SIZE);
        assert(Obj8::GetAliveObj8ectCount() == SIZE);
    }
}

void Test8() {
    const size_t MEDIUM_SIZE = 100;
    const size_t LARGE_SIZE = 250;
    const int ID = 42;
    {
        Obj8::ResetCounters();
        Vector<int> v(MEDIUM_SIZE);
        {
            auto v_copy(std::move(v));

            assert(v_copy.Size() == MEDIUM_SIZE);
            assert(v_copy.Capacity() == MEDIUM_SIZE);
        }
        assert(Obj8::GetAliveObj8ectCount() == 0);
    }
    {
        Obj8::ResetCounters();
        {
            Vector<Obj8> v(MEDIUM_SIZE);
            v[MEDIUM_SIZE / 2].id = ID;
            assert(Obj8::num_default_constructed == MEDIUM_SIZE);
            Vector<Obj8> moved_from_v(std::move(v));
            assert(moved_from_v.Size() == MEDIUM_SIZE);
            assert(moved_from_v[MEDIUM_SIZE / 2].id == ID);
        }
        assert(Obj8::GetAliveObj8ectCount() == 0);

        assert(Obj8::num_moved == 0);
        assert(Obj8::num_copied == 0);
        assert(Obj8::num_default_constructed == MEDIUM_SIZE);
    }
    {
        Obj8::ResetCounters();
        Vector<Obj8> v_medium(MEDIUM_SIZE);
        v_medium[MEDIUM_SIZE / 2].id = ID;
        Vector<Obj8> v_large(LARGE_SIZE);
        v_large = v_medium;
        assert(v_large.Size() == MEDIUM_SIZE);
        assert(v_large.Capacity() == LARGE_SIZE);
        assert(v_large[MEDIUM_SIZE / 2].id == ID);
        assert(Obj8::GetAliveObj8ectCount() == MEDIUM_SIZE + MEDIUM_SIZE);
    }
    {
        Obj8::ResetCounters();
        Vector<Obj8> v(MEDIUM_SIZE);
        {
            Vector<Obj8> v_large(LARGE_SIZE);
            v_large[LARGE_SIZE - 1].id = ID;
            v = v_large;
            assert(v.Size() == LARGE_SIZE);
            assert(v_large.Capacity() == LARGE_SIZE);
            assert(v_large[LARGE_SIZE - 1].id == ID);
            assert(Obj8::GetAliveObj8ectCount() == LARGE_SIZE + LARGE_SIZE);
        }
        assert(Obj8::GetAliveObj8ectCount() == LARGE_SIZE);
    }
    assert(Obj8::GetAliveObj8ectCount() == 0);
    {
        Obj8::ResetCounters();
        Vector<Obj8> v(MEDIUM_SIZE);
        v[MEDIUM_SIZE - 1].id = ID;
        Vector<Obj8> v_small(MEDIUM_SIZE / 2);
        v_small.Reserve(MEDIUM_SIZE + 1);
        const size_t num_copies = Obj8::num_copied;
        v_small = v;
        assert(v_small.Size() == v.Size());
        assert(v_small.Capacity() == MEDIUM_SIZE + 1);
        v_small[MEDIUM_SIZE - 1].id = ID;
        assert(Obj8::num_copied - num_copies == MEDIUM_SIZE - (MEDIUM_SIZE / 2));
    }
}