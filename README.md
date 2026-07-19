# LoggerLib (библиотека логирования)

---

## О проекте
Текст

---

## Структура репозитория
```
├── lib/                          # библиотека логирования
│   ├── include/
│   │   └── logger/
│   │       ├── logger.hpp        # публичный класс Logger
│   │       └── strategies/ 
│   │              ├── log_strategy.hpp         # интерфейс ILogStrategy (Strategy pattern)
│   │              ├── file_log_strategy.hpp    # запись в файл
│   │              └── socket_log_strategy.hpp  # запись в сокет
│   ├── src/
│   │   ├── logger.cpp
│   │   ├── strategies/
│   │   │   ├── file_log_strategy.cpp    # запись в файл
│   │   │   └── socket_log_strategy.cpp  # запись в сокет
│   │   └── utils/
│   │       └── error_utils.hpp / .cpp          # потокобезопасное чтение errno
│   └── CMakeLists.txt
├── app/                           # консольное приложение
│   ├── src/
│   └── CMakeLists.txt
├── tests/                         # юнит-тесты
│   └── CMakeLists.txt
├── CMakeLists.txt                 # корневой файл сборки
└── README.md
```
`include/` содержит только публичный API библиотеки — то, что видит и использует приложение. Всё в `src/`, включая внутренние утилиты (`utils/`), — детали реализации, недоступные снаружи библиотеки.

---

## Сборка

Зависимости:
- компилятор с поддержкой C++17 (проверено на gcc)
- CMake ≥ 3.10
- POSIX-совместимая ОС (проверено на Ubuntu/Debian) — используются POSIX sockets и `pthread`

Библиотека может быть собрана как статическая (по умолчанию) или как динамическая — переключается флагом `BUILD_SHARED_LIBS`:

```bash
# статическая сборка (по умолчанию)
cmake -B build -S .
cmake --build build

# динамическая сборка
cmake -B build -S . -DBUILD_SHARED_LIBS=ON
cmake --build build
```

Библиотека и приложение собираются как раздельные CMake-таргеты в одном проходе; после сборки исполняемый файл приложения находится в `build/app/`, библиотека — в `build/lib/`.

---

## Использование библиотеки

#### Инициализация с записью в файл:

```cpp
#include <logger/logger.hpp>

Logger logger("app.log", MessageLevel::INFO);

logger.log("Server started");                      // с уровнем по умолчанию (INFO)
logger.log("Connection refused", MessageLevel::ERROR);

logger.change_default_level(MessageLevel::WARNING); // сообщения ниже WARNING больше не пишутся
```

#### Инициализация с записью в сокет 
(тот же интерфейс `log()`, другой backend передаётся при создании)

```cpp
Logger logger("127.0.0.1", 9000, MessageLevel::DEBUG);
logger.log("Connected to stats collector");
```

Если файл/сокет не удалось открыть — `Logger` не бросает исключение (по требованиям ТЗ), 
а выводит причину в `stderr` при создании и на каждой последующей попытке записи; 
сама программа продолжает работать.

#### Прямое использование стратегий

Конкретные реализации `ILogStrategy` (`FileLogStrategy`, `SocketLogStrategy`) 
— часть публичного API и находятся в `include/logger/strategies/`. 
Это позволяет собрать и проверить стратегию до создания `Logger`, 
если нужен более тонкий контроль над обработкой ошибок инициализации:

```cpp
#include <logger/strategies/file_log_strategy.hpp>

std::string err;
auto strategy = FileLogStrategy::create("app.log", err);
if (!strategy) {
    // своя обработка ошибки — например, fallback на другой файл или завершение программы
}

Logger logger(std::move(strategy), MessageLevel::INFO);
```

---

## Использование консольного приложения

Текст

---

## Архитектурные решения

Текст

---

## Тестирование

Текст

---

## Ограничения

Текст