## Задание

Сделайте конструкторы класса `Vector` и метод `Reserve` устойчивыми к возникновению исключений. Исключения не должны приводить к утечкам памяти или неопределённому поведению. Метод `Reserve` в случае возникновения исключения должен оставлять вектор в прежнем состоянии.

Чтобы проверить, что исключения обрабатываются корректно, используйте тесты из файла ниже:

- `tests.cpp`

```
  #include "vector.h"#include <stdexcept>namespace {

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

      Obj11() {
          ++num_destroyed;
      }

      static int GetAliveObjectCount() {
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
          assert(Obj1::GetAliveObjectCount() == 0);
      }
      {
          Vector<Obj1> v(SIZE);
          assert(Obj1::GetAliveObjectCount() == SIZE);
          v.Reserve(SIZE * 2);
          assert(Obj1::GetAliveObjectCount() == SIZE);
      }
      assert(Obj1::GetAliveObjectCount() == 0);
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
          assert(Obj1::GetAliveObjectCount() == 0);
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
          assert(Obj1::GetAliveObjectCount() == SIZE);
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
          assert(Obj1::GetAliveObjectCount() == SIZE);
      }
  }

  int main() {
      Test1();
      Test2();
  }

```

### Что отправлять на проверку

Исходный код класса `Vector` должен располагаться в файле `vector.h`.

### Ограничения

Реализуйте класс `Vector`, не используя класс `std::vector` или его аналоги.

Методы класса `Vector` не должны ничего выводить в `stdin` и `stdout`. В противном случае решение принято не будет.

### Как будет тестироваться ваш код

Работа методов класса `Vector` и корректность обработки исключений будут проверены тестами, подобными тем, что представлены в файле `tests.cpp`.

Не изменяйте сигнатуру публичных методов класса `Vector`. В противном случае тренажёр может отклонить решение.

Чтобы проверить работу с указателями и динамической памятью, код будет скомпилирован со включёнными UB и Address санитайзерами.

Функция `main` будет заменена кодом из тренажёра.

### Доп

В методе `Reserve`

обработчик исключения должен разрушить объекты, скопированные в новую
область памяти, а затем освободить выделенную под них память и
перевыбросить пойманное исключение.