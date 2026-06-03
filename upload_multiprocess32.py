import os
import subprocess
from threading import Thread
Import("env")

# 1. Укажи твои порты здесь
TARGET_PORTS = ["COM8", "COM6"] 

def run_esptool(port, firmware_path, bootloader_bin, partitions_bin):
    # ВЫТАСКИВАЕМ РАБОЧИЙ ПРОШИВАЛЬЩИК ИЗ САМОГО PLATFORMIO
    # env.subst("$UPLOADER") возвращает точный путь, которым шьет сама IDE
    uploader = env.subst("$UPLOADER")
    
    # Если PlatformIO использует esptool.py, то $UPLOADER будет содержать путь к нему.
    # Но для выполнения скрипта на Windows нужен Python. 
    # Проверяем: если uploader заканчивается на .py, добавим перед ним рабочий python
    if uploader.endswith(".py"):
        cmd = [env.subst("$PYTHONEXE"), uploader]
    else:
        cmd = [uploader]

    # Накидываем стандартные аргументы для ESP32
    cmd.extend([
        "--chip", "esp32",
        "--port", port,
        "--baud", "921600",
        "--before", "default_reset",
        "--after", "hard_reset",
        "write_flash", "-z",
        "--flash_mode", "dio",
        "--flash_freq", "40m",
        "--flash_size", "detect",
        "0x1000", bootloader_bin,
        "0x8000", partitions_bin,
        "0x10000", firmware_path
    ])
    
    print(f"--> [Старт] Прошивка ESP32 на порту {port}...\n")
    
    # Запускаем через shell=True на случай, если в путях PlatformIO есть пробелы
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    
    if result.returncode == 0:
        print(f" [УСПЕХ] ESP32 на порту {port} успешно прошита!\n")
    else:
        print(f" [ОШИБКА] Порт {port} завершился с ошибкой:\n{result.stderr}\n")

def parallel_upload(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    firmware_path = os.path.join(build_dir, "firmware.bin")
    bootloader_bin = os.path.join(build_dir, "bootloader.bin")
    partitions_bin = os.path.join(build_dir, "partitions.bin")
    
    if not os.path.exists(firmware_path):
        print(f" [ОШИБКА] Файл прошивки не найден. Сначала скомпилируй проект!")
        return

    print(f"\nЗапуск параллельной прошивки через системный Uploader на порты: {TARGET_PORTS}\n")

    threads = []
    for port in TARGET_PORTS:
        t = Thread(target=run_esptool, args=(port, firmware_path, bootloader_bin, partitions_bin))
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

    print("\n[Все параллельные потоки прошивки завершены!]")

# Регистрируем цель
env.AddCustomTarget(
    name="multi32",
    dependencies=["$BUILD_DIR/${PROGNAME}.bin"], 
    actions=parallel_upload,
    title="Parallel Upload",
    description="Параллельный запуск родного загрузчика PlatformIO"
)