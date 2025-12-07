# URL Shortener

Простой сервис для сокращения URL, написанный на C++ с использованием Crow и SQLite.

## Функциональность

- Сокращение URL: POST /shorten с JSON {"url": "http://example.com"}
- Перенаправление: GET /<short_code>
- Удаление: DELETE /delete/<short_code>

## Требования

- CMake
- C++ компилятор
- SQLite3
- Google Test (для тестирования)

## Сборка

```bash
mkdir build
cd build
cmake ..
make
```

## Запуск

```bash
./url_shortener
```

Сервер запустится на порту 8080.

## Тестирование

После сборки:

```bash
make test
# или
ctest
# или напрямую
./url_shortener_tests
```

## Использование с Docker

```bash
docker-compose up
```

## Конфигурация

Создайте файл `config.txt` с настройками:

```
short_code_length=6
```

По умолчанию длина короткого кода - 6 символов.

## API

### Сокращение URL

POST /shorten

Тело запроса (JSON):
```json
{
  "url": "http://example.com"
}
```

Ответ:
```json
{
  "short_url": "http://localhost:8080/abc123"
}
```

### Перенаправление

GET /<short_code>

Перенаправляет на оригинальный URL.

### Удаление

DELETE /delete/<short_code>

Удаляет короткий URL.