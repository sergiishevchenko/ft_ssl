# Запуск ft_ssl в виртуальной машине

Задание требует Linux. На macOS проект можно собрать и запустить локально, но проверка `md5sum`, `sha256sum` и `valgrind` делается в Linux-виртуалке.

Ниже — путь от пустой машины до прогона примеров из subject.

## 1. Что понадобится

- Свободно около 20 ГБ на диске и 2 ГБ оперативной памяти под гостевую систему.
- Образ Ubuntu 24.04 LTS (Desktop или Server): https://ubuntu.com/download/desktop
- Гипервизор:
  - Mac с Apple Silicon — [UTM](https://mac.getutm.app/)
  - Mac с Intel или Windows — [VirtualBox](https://www.virtualbox.org/)

Минимальные параметры виртуалки:

| Параметр | Значение |
|---|---|
| ОС | Ubuntu 24.04 LTS, 64-bit |
| RAM | 2048 МБ |
| CPU | 2 ядра |
| Диск | 20 ГБ, динамический |
| Сеть | NAT (интернет из коробки) |

## 2. Создать виртуалку

### UTM (Apple Silicon)

1. Открой UTM → **Create a New Virtual Machine** → **Virtualize**.
2. Выбери **Linux**.
3. Укажи скачанный ISO Ubuntu.
4. Поставь 2048 МБ RAM и 2 ядра, диск 20 ГБ.
5. Запусти машину и пройди установщик Ubuntu: язык, пользователь, пароль, установка на весь диск виртуалки.
6. После перезагрузки извлеки ISO: в настройках машины убери загрузочный образ, иначе установка начнётся снова.

### VirtualBox

1. **Создать** → имя `ft_ssl`, тип Linux, версия Ubuntu (64-bit).
2. RAM 2048 МБ, виртуальный диск VDI 20 ГБ, динамический.
3. Настройки машины → **Носители** → пустой оптический привод → выбери ISO Ubuntu.
4. **Сеть** → адаптер 1 → NAT.
5. Запусти, установи Ubuntu так же, как выше.
6. После установки: **Устройства → Оптические диски → Извлечь диск из привода**, затем перезагрузка.

## 3. Поставить инструменты в гостевой Ubuntu

Открой терминал внутри виртуалки и выполни:

```bash
sudo apt update
sudo apt install -y build-essential git coreutils openssl valgrind
```

Проверка:

```bash
cc --version
make --version
md5sum --version
sha256sum --version
valgrind --version
```

`cc` и `make` нужны для сборки. `md5sum` и `sha256sum` — эталон для сверки хешей. `valgrind` — проверка утечек, её требует план проекта и её нельзя нормально прогнать на macOS.

OpenSSL 3 на Ubuntu не считает Whirlpool (`openssl dgst -whirlpool` вернёт ошибку). Эталон для Whirlpool — тестовые векторы из README, не системный `openssl`.

## 4. Перенести проект в виртуалку

Любой один из способов.

### Через git

Если репозиторий уже на GitHub или внутришкольном git:

```bash
cd ~
git clone <URL-репозитория> ft_ssl
cd ft_ssl
```

### Через общую папку

**UTM:** настройки выключенной машины → **Sharing** → включи directory share и укажи папку проекта на Mac. В Ubuntu она обычно появляется в `/media/share` или через SPICE. Скопируй её к себе:

```bash
cp -R /media/share/ft_ssl ~/ft_ssl
cd ~/ft_ssl
```

**VirtualBox:** установи Guest Additions (**Устройства → Подключить образ Дополнений гостевой ОС**), затем **Настройки → Общие папки** → добавь папку проекта, поставь «Автоподключение». В Ubuntu:

```bash
sudo usermod -aG vboxsf "$USER"
```

Выйди из сессии и зайди снова. Папка будет в `/media/sf_<имя>`.

### Через scp с хоста

На виртуалке узнай IP (`ip -4 addr`). При NAT с хоста напрямую не достучаться: в VirtualBox добавь проброс порта **Сеть → Адаптер 1 → Расширенные → Проброс портов**: хост `2222` → гость `22`. Затем с Mac:

```bash
scp -P 2222 -r "/путь/к/ft_ssl [md5][sha256]" user@127.0.0.1:~/ft_ssl
```

В имени каталога есть пробелы и скобки — путь на Mac всегда в кавычках.

## 5. Собрать

В каталоге проекта внутри виртуалки:

```bash
cd ~/ft_ssl
make
```

Ожидается исполняемый файл `ft_ssl` без предупреждений (`-Wall -Wextra -Werror`).

Остальные цели:

```bash
make clean    # удалить .o
make fclean   # удалить .o и ft_ssl
make re       # пересобрать с нуля
```

## 6. Прогнать обязательную часть

Подготовь файл из subject и сравни вывод.

```bash
echo "And above all," > file

echo "42 is nice" | ./ft_ssl md5
# (stdin)= 35f1d6de0302e2086a4e472266efb3a9

echo "42 is nice" | ./ft_ssl md5 -p
# ("42 is nice")= 35f1d6de0302e2086a4e472266efb3a9

echo "Pity the living." | ./ft_ssl md5 -q -r
# e20c3b973f63482a778f3fd1869b7f25

./ft_ssl md5 file
# MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a

./ft_ssl md5 -r file
# 53d53ea94217b259c11a5a2d104ec58a file

./ft_ssl md5 -s "pity those that aren't following baerista on spotify."
# MD5 ("...") = a3c990a1964705d9bf0e602f44572f5f

echo "be sure to handle edge cases carefully" | ./ft_ssl md5 -p file
echo "some of this will not make sense at first" | ./ft_ssl md5 file
echo "but eventually you will understand" | ./ft_ssl md5 -p -r file
echo "GL HF let's go" | ./ft_ssl md5 -p -s "foo" file
echo "one more thing" | ./ft_ssl md5 -r -p -s "foo" file -s "bar"
echo "just to be extra clear" | ./ft_ssl md5 -r -q -p -s "foo" file
```

Сверка с системными утилитами. `echo` добавляет перевод строки, для строки без `\n` используй `echo -n` или `printf`.

```bash
printf '42 is nice' | ./ft_ssl md5
printf '42 is nice' | md5sum

echo "https://www.42.fr/" > website
./ft_ssl sha256 -q website
sha256sum website

./ft_ssl sha256 -s "42 is nice"
printf '42 is nice' | sha256sum
```

Хеш в первой колонке `md5sum` / `sha256sum` должен совпасть с выводом `ft_ssl`. Формат строки (`(stdin)=`, `MD5 (file) =`) — свой, как в subject, и с `md5sum` его сравнивать не нужно.

Неизвестная команда:

```bash
./ft_ssl foobar
```

Ожидается сообщение `ft_ssl: Error: 'foobar' is an invalid command.` и списки `Commands` и `Flags`.

## 7. Прогнать бонус

Интерактивный режим — запуск без аргументов. Сначала печатается `usage`, затем приглашение `ft_ssl>`.

```bash
./ft_ssl
```

```text
usage: ft_ssl command [flags] [file/string]
ft_ssl> md5 -s "hello"
ft_ssl> sha256 -s "hello"
ft_ssl> whirlpool -s "abc"
```

Выход: `Ctrl+D`.

Whirlpool на пустой строке:

```bash
./ft_ssl whirlpool -s ""
```

Ожидаемый дайджест:

```text
19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3
```

## 8. Проверить утечки

```bash
echo "42 is nice" | valgrind --leak-check=full --show-leak-kinds=all ./ft_ssl md5
./ft_ssl md5 -s "hello" file
valgrind --leak-check=full ./ft_ssl sha256 -s "42 is nice"
```

В конце отчёта должно быть `All heap blocks were freed -- no leaks are possible` и `ERROR SUMMARY: 0 errors`. Сообщение про несуществующий `file` — нормальное поведение программы, не ошибка valgrind.

Для интерактива передай команды через пайп, иначе valgrind будет ждать ввод:

```bash
printf 'md5 -s "hello"\n' | valgrind --leak-check=full ./ft_ssl
```
