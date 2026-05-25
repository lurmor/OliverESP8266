import os
import subprocess
from threading import Thread # Заменили Process на Thread
Import("env")

# 1. Укажите ваши порты здесь
TARGET_PORTS = ["COM3", "COM5"] 

def run_esptool(port, firmware_path):
    # Достаем точный путь к python внутри PlatformIO, чтобы найти esptool
    # Это избавит нас от необходимости жестко прописывать пути вручную
    python_exe = env.subst("$PYTHONEXE")
    penv_dir = os.path.dirname(python_exe)
    pio_exe = os.path.join(penv_dir, "pio.exe")

    # Если pio.exe в этой папке нет (зависит от версии), берем прямую команду esptool
    cmd = [
        pio_exe, "pkg", "exec", "-p", "tool-esptoolpy", "--",
        "esptool.py",
        "--chip", "esp8266",
        "--port", port,
        "--baud", "921600",
        "write_flash", "0x0", firmware_path
    ]
    
    print(f"--> [Старт] Прошивка платы на порту {port}...\n")
    
    # Флаг shell=True обязателен для Windows, чтобы правильно разрешать пути исполняемых файлов
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    
    if result.returncode == 0:
        print(f" [УСПЕХ] Плата на порту {port} успешно прошита!\n")
    else:
        print(f" [ОШИБКА] Порт {port} завершился с ошибкой:\n{result.stderr}\n")

def parallel_upload(source, target, env):
    # Надежно достаем путь к собранному файлу прошивки
    firmware_path = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
    
    if not os.path.exists(firmware_path):
        print(f" [ОШИБКА] Файл прошивки не найден по адресу: {firmware_path}")
        print("Сначала соберите проект (нажмите Build / Галочку)!")
        return

    print(f"\n Найдена прошивка: {firmware_path}")
    print(f" Запуск параллельной прошивки на порты: {TARGET_PORTS}\n")

    threads = []
    # Запускаем параллельные ПОТОКИ вместо процессов
    for port in TARGET_PORTS:
        t = Thread(target=run_esptool, args=(port, firmware_path))
        t.start()
        threads.append(t)

    # Ждем, пока все потоки дошьют свои платы
    for t in threads:
        t.join()

    print("\n[Все параллельные потоки прошивки завершены!]")

# Регистрируем кастомную цель загрузки "multi"
env.AddCustomTarget(
    name="multi",
    dependencies=["$BUILD_DIR/${PROGNAME}.bin"], 
    actions=parallel_upload,
    title="Parallel Upload",
    description="Прошивка всех плат одновременно через потоки"
)