# Проверка ft_ssl

На macOS проект собирается, и хеши совпадают с системными. Полный прогон удобнее делать в Linux: там есть `md5sum`, `sha256sum` и `valgrind`.

Ниже — путь от пустой машины до проверки сборки, формата вывода, хешей, ошибок, интерактива и утечек.

Хеш сравнивай с `md5sum` / `sha256sum` (первая колонка). Строку целиком (`(stdin)=`, `MD5 (file) =`) с ними не сравнивай: формат у `ft_ssl` свой.

## 1. Что понадобится

- Около 20 ГБ на диске и 2 ГБ RAM под гостевую систему.
- Ubuntu 24.04 LTS: https://ubuntu.com/download/desktop
- Гипервизор:
  - Mac с Apple Silicon — [UTM](https://mac.getutm.app/)
  - Mac с Intel или Windows — [VirtualBox](https://www.virtualbox.org/)

| Параметр | Значение |
|---|---|
| ОС | Ubuntu 24.04 LTS, 64-bit |
| RAM | 2048 МБ |
| CPU | 2 ядра |
| Диск | 20 ГБ, динамический |
| Сеть | NAT |

## 2. Создать виртуалку

### UTM (Apple Silicon)

1. UTM → **Create a New Virtual Machine** → **Virtualize**.
2. **Linux**, укажи ISO Ubuntu.
3. 2048 МБ RAM, 2 ядра, диск 20 ГБ.
4. Установи Ubuntu на весь диск виртуалки.
5. После перезагрузки убери ISO из настроек, иначе установщик запустится снова.

### VirtualBox

1. **Создать** → имя `ft_ssl`, тип Linux, Ubuntu (64-bit).
2. RAM 2048 МБ, диск VDI 20 ГБ, динамический.
3. **Носители** → оптический привод → ISO Ubuntu.
4. **Сеть** → адаптер 1 → NAT.
5. Установи Ubuntu. Затем **Устройства → Оптические диски → Извлечь диск**, перезагрузка.

## 3. Инструменты в Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential git coreutils openssl valgrind
```

```bash
cc --version
make --version
md5sum --version
sha256sum --version
valgrind --version
```

`openssl dgst -whirlpool` на OpenSSL 3 в Ubuntu не работает. Для Whirlpool ориентир — векторы в разделе 10.

## 4. Перенести проект

Любой один способ.

### git

```bash
cd ~
git clone <URL-репозитория> ft_ssl
cd ft_ssl
```

### Общая папка

**UTM:** выключенная машина → **Sharing** → папка проекта на Mac. В Ubuntu обычно `/media/share`:

```bash
cp -R /media/share/ft_ssl ~/ft_ssl
cd ~/ft_ssl
```

**VirtualBox:** Guest Additions, затем **Общие папки**, автоподключение:

```bash
sudo usermod -aG vboxsf "$USER"
```

Выйди из сессии и зайди снова. Папка будет в `/media/sf_<имя>`.

### scp

На виртуалке: `ip -4 addr`. При NAT в VirtualBox пробрось порт: хост `2222` → гость `22`. С Mac:

```bash
scp -P 2222 -r "/путь/к/ft_ssl [md5][sha256]" user@127.0.0.1:~/ft_ssl
```

В имени каталога на Mac есть пробелы и скобки — путь в кавычках.

## 5. Собрать

```bash
cd ~/ft_ssl
make
```

Ожидается `ft_ssl` без предупреждений (`-Wall -Wextra -Werror`).

```bash
make clean    # удалить .o
make fclean   # удалить .o и ft_ssl
make re       # пересобрать с нуля
```

## 6. Формат вывода

Подготовь файл:

```bash
echo "And above all," > file
```

Дальше вывод должен совпасть построчно.

```bash
echo "42 is nice" | ./ft_ssl md5
```

```text
(stdin)= 35f1d6de0302e2086a4e472266efb3a9
```

```bash
echo "42 is nice" | ./ft_ssl md5 -p
```

```text
("42 is nice")= 35f1d6de0302e2086a4e472266efb3a9
```

```bash
echo "Pity the living." | ./ft_ssl md5 -q -r
```

```text
e20c3b973f63482a778f3fd1869b7f25
```

```bash
./ft_ssl md5 file
./ft_ssl md5 -r file
```

```text
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a
53d53ea94217b259c11a5a2d104ec58a file
```

```bash
./ft_ssl md5 -s "pity those that aren't following baerista on spotify."
```

```text
MD5 ("pity those that aren't following baerista on spotify.") = a3c990a1964705d9bf0e602f44572f5f
```

```bash
echo "be sure to handle edge cases carefully" | ./ft_ssl md5 -p file
```

```text
("be sure to handle edge cases carefully")= 3553dc7dc5963b583c056d1b9fa3349c
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a
```

```bash
echo "some of this will not make sense at first" | ./ft_ssl md5 file
```

```text
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a
```

Пайп без `-p` игнорируется, хешируется только файл.

```bash
echo "but eventually you will understand" | ./ft_ssl md5 -p -r file
```

```text
("but eventually you will understand")= dcdd84e0f635694d2a943fa8d3905281
53d53ea94217b259c11a5a2d104ec58a file
```

```bash
echo "GL HF let's go" | ./ft_ssl md5 -p -s "foo" file
```

```text
("GL HF let's go")= d1e3cc342b6da09480b27ec57ff243e2
MD5 ("foo") = acbd18db4cc2f85cedef654fccc4a4d8
MD5 (file) = 53d53ea94217b259c11a5a2d104ec58a
```

```bash
echo "one more thing" | ./ft_ssl md5 -r -p -s "foo" file -s "bar"
```

```text
("one more thing")= a0bd1876c6f011dd50fae52827f445f5
acbd18db4cc2f85cedef654fccc4a4d8 "foo"
53d53ea94217b259c11a5a2d104ec58a file
ft_ssl: md5: -s: No such file or directory
ft_ssl: md5: bar: No such file or directory
```

После первого не-флага всё остальное — имена файлов, в том числе второе `-s`.

```bash
echo "just to be extra clear" | ./ft_ssl md5 -r -q -p -s "foo" file
```

```text
just to be extra clear
3ba35f1ea0d170cb3b9a752e3360286c
acbd18db4cc2f85cedef654fccc4a4d8
53d53ea94217b259c11a5a2d104ec58a
```

```bash
echo "https://www.42.fr/" > website
./ft_ssl sha256 -q website
./ft_ssl sha256 -s "42 is nice"
```

```text
1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf
SHA256 ("42 is nice") = b7e44c7a40c5f80139f0a50f3650fb2bd8d00b0d24667c4c2ca32c88e13b758f
```

## 7. Сверка хеша с системой

`echo` добавляет `\n`. Для строки без перевода строки — `printf`.

```bash
echo "42 is nice" | ./ft_ssl md5 -q
echo "42 is nice" | md5sum

printf '42 is nice' | ./ft_ssl md5 -q
printf '42 is nice' | md5sum

./ft_ssl sha256 -q website
sha256sum website

./ft_ssl sha256 -q -s "42 is nice"
printf '42 is nice' | sha256sum
```

Первая колонка `md5sum` / `sha256sum` должна совпасть с `-q` у `ft_ssl`.

Пустая строка и короткие векторы:

```bash
./ft_ssl md5 -q -s ""
./ft_ssl md5 -q -s "abc"
./ft_ssl sha256 -q -s ""
./ft_ssl sha256 -q -s "abc"
```

```text
d41d8cd98f00b204e9800998ecf8427e
900150983cd24fb0d6963f7d28e17f72
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
```

## 8. Ошибки

```bash
./ft_ssl foobar
```

```text
ft_ssl: Error: 'foobar' is an invalid command.
Commands:
md5
sha256
whirlpool
Flags:
-p -q -r -s
```

Код возврата ненулевой.

```bash
./ft_ssl md5 .
./ft_ssl md5 -s
```

```text
ft_ssl: md5: .: Is a directory
ft_ssl: md5: option requires an argument -- s
```

Каталог не хешируется как пустой файл.

## 9. Команды со stdin

Без аргументов программа читает команды со stdin. Приглашение `ft_ssl> ` только если stdin — терминал. Через пайп приглашения нет.

Интерактив (TTY):

```bash
./ft_ssl
```

```text
ft_ssl> md5 -s "42 is nice"
MD5 ("42 is nice") = 0029a98ee90fdb85d70924d44d3c9e75
ft_ssl> sha256 -q website
1ceb55d2845d9dd98557b50488db12bbf51aaca5aa9c1199eb795607a2457daf
ft_ssl> foobar
ft_ssl: Error: 'foobar' is an invalid command.
Commands:
md5
sha256
whirlpool
Flags:
-p -q -r -s
ft_ssl> exit
```

Пустая строка ничего не делает. Неизвестная команда не завершает программу. Выход: `exit`, `quit` или Ctrl-D.

Пайп:

```bash
printf 'md5 -s "42 is nice"\nsha256 -s "42 is nice"\n' | ./ft_ssl
```

```text
MD5 ("42 is nice") = 0029a98ee90fdb85d70924d44d3c9e75
SHA256 ("42 is nice") = b7e44c7a40c5f80139f0a50f3650fb2bd8d00b0d24667c4c2ca32c88e13b758f
```

Пустая строка, ошибка, выход:

```bash
printf '\nfoobar\nexit\n' | ./ft_ssl
```

Сначала ошибка на `foobar`, затем выход по `exit`. Приглашения нет.

## 10. Whirlpool

Те же флаги `-p -q -r -s`. Векторы ISO/IEC 10118-3:

```bash
./ft_ssl whirlpool -q -s ""
./ft_ssl whirlpool -q -s "a"
./ft_ssl whirlpool -q -s "abc"
```

```text
19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f1280f9ac12dce4ecd82c5be59e401b8e501d
8aca2602792aec6f11a67206531fb8ac4a2a76f9ad18c47a7541565973c0a04d0d5f13e688b6abd56c006191ff52204943301740c46207ea21bd67a3363bac65
4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cb70c4afc72894b03123cec4a76917a3e826
```

Формат без `-q` такой же, как у md5 и sha256:

```bash
./ft_ssl whirlpool -s "abc"
```

```text
WHIRLPOOL ("abc") = 4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cb70c4afc72894b03123cec4a76917a3e826
```

## 11. Утечки

`valgrind` — только Linux.

```bash
echo "42 is nice" | valgrind --leak-check=full --show-leak-kinds=all ./ft_ssl md5
valgrind --leak-check=full ./ft_ssl sha256 -s "42 is nice"
valgrind --leak-check=full ./ft_ssl whirlpool -s "abc"
printf 'md5 -s "hello"\n' | valgrind --leak-check=full ./ft_ssl
```

В конце: `All heap blocks were freed -- no leaks are possible` и `ERROR SUMMARY: 0 errors`.

Сообщение `No such file or directory` при несуществующем файле — вывод программы, не ошибка valgrind:

```bash
valgrind --leak-check=full ./ft_ssl md5 -s "hello" missing
```

Интерактив без пайпа valgrind будет ждать ввод — всегда корми команды через `printf`.

## 12. Убрать тестовые файлы

```bash
rm -f file website
```
