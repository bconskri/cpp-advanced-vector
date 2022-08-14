## Задание

Сохраните своё решение. Оно вам ещё понадобится в следующих уроках.

Реализуйте в классе `Vector` методы `Resize`, `PushBack` и `PopBack`.

Обе версии метода `PushBack` должны поддерживать корректную вставку элемента вектора в конец этого же вектора.

Сигнатура методов:

```
template <typename T>
class Vector {
public:
    void Resize(size_t new_size);
    void PushBack(const T& value);
    void PushBack(T&& value);
    void PopBack() /* noexcept */;
    ...
};

```

Метод `PushBack` должен предоставлять строгую гарантию безопасности исключений, когда выполняется любое из условий:

- мove-конструктор у типа `T` объявлен как `noexcept`;
- тип `T` имеет публичный конструктор копирования.

Если у типа `T` нет конструктора копирования и move-конструктор может выбрасывать исключения, метод `PushBack` должен предоставлять базовую гарантию безопасности исключений.

Сложность метода `PushBack` должна быть амортизированной константой.

Метод `PopBack` не должен выбрасывать исключений при вызове у непустого вектора. При вызове `PopBack` у пустого вектора поведение неопределённо.

Метод `PopBack` должен иметь сложность $O(1)$.

Метод `Resize` должен предоставлять строгую гарантию безопасности исключений, когда выполняется любое из условий:

- move-конструктор у типа `T` объявлен как `noexcept`;
- тип `T` имеет публичный конструктор копирования.

Если у типа `T` нет конструктора копирования и move-конструктор может выбрасывать исключения, метод `Resize` может предоставлять базовую или строгую гарантию безопасности исключений.

Сложность метода `Resize`
должна линейно зависеть от разницы между текущим и новым размером
вектора. Если новый размер превышает текущую вместимость вектора,
сложность операции может дополнительно линейно зависеть от текущего
размера вектора.

- Что отправлять на проверку

Исходный код классов `Vector` и `RawMemory` должен располагаться в файле `vector.h`.

### Как будет тестироваться ваша программа

Тренажёр проверит работу методов класса `Vector`, включая разработанные в этом задании методы `Resize`, `PushBack` и `PopBack`.

Не меняйте сигнатуру публичных методов класса `Vector`. Иначе решение может быть не принято.

Так как вызов `PopBack`
на пустом векторе может привести к неопределённому поведению, тренажёр
не будет вызывать этот метод у пустого вектора. Добавление спецификатора
`noexcept` у этого метода остаётся на ваше усмотрение.

Код
программы будет скомпилирован со включёнными UB и Address
санитайзерами, чтобы выявить потенциальные проблемы при работе с
указателями и динамической памятью.

Функция `main` будет заменена кодом из тренажёра.

+

- Если у вас сложности — вернитесь к схемам в уроке.
- Подумайте, как можно устранить дублирование кода двумя версиями `PushBack`.