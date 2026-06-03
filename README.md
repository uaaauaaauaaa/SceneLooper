# Atmocycle (Multi-Layer Ambience Looper)

**Atmocycle** — это профессиональный многослойный аудиоплагин для создания атмосферных подкладов, текстур и цикличных звуковых ландшафтов. Плагин поддерживает форматы VST3, AU и AAX на платформах macOS и Windows.

## Основные возможности (Features)
- **8 независимых звуковых слоев (WAV)** с поддержкой файлов частотой 48 кГц / 24-бит.
- **Индивидуальное управление слоями**: громкость (volume), панорама (pan), фильтры высоких (HP) и низких (LP) частот, кроссфейды и смещение старта (start offset).
- **Режимы On/Solo**: быстрое включение, отключение и солирование каждого слоя.
- **Интеллектуальный Автопилот (Autopilot)**: система случайных переходов внутри слоев для создания бесконечной непредсказуемой атмосферы.
- **Рандомизация (Randomization)**: кнопка для быстрого сброса и случайного изменения фаз и точек старта слоев.
- **Сохранение и загрузка пресетов/сцен**: возможность сохранять состояние проекта во внешний XML-файл и восстанавливать его.
- **Современный интерфейс**: дизайн в стиле Glassmorphism с динамическими индикаторами уровня звука (VU-метрами) для мастер-канала (левый/правый) и каждого отдельного слоя.

## Системные требования (System Requirements)
- **macOS**: 10.15 (Catalina) и новее (Apple Silicon M1/M2/M3 & Intel x64). Форматы: VST3, AU, AAX.
- **Windows**: 10, 11 (x64). Форматы: VST3, AAX.

---

## Руководство по сборке (Build Instructions)

Для сборки плагина требуется установленный **CMake** (версии 3.22 и выше) и соответствующий компилятор (Xcode на macOS или MSVC / Visual Studio на Windows). Зависимости (JUCE 8.0.12) загружаются автоматически через CMake `FetchContent`.

### Сборка на macOS
Откройте терминал в папке проекта и выполните:
```bash
# Конфигурация проекта (Release)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Сборка всех форматов (VST3, AU, AAX)
cmake --build build --config Release --parallel

# Готовые плагины будут скопированы в системные папки:
# - VST3: ~/Library/Audio/Plug-Ins/VST3/Atmocycle.vst3
# - AU: ~/Library/Audio/Plug-Ins/Components/Atmocycle.component
# - AAX: /Library/Application Support/Avid/Audio/Plug-Ins/Atmocycle.aaxplugin
```

### Сборка на Windows
Откройте командную строку разработчика (Developer PowerShell/Command Prompt для VS) и выполните:
```powershell
# Конфигурация проекта (Release)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Сборка всех форматов (VST3, AAX)
cmake --build build --config Release --parallel
```

---

## Что нужно для полноценного выпуска плагина (Production checklist)

Чтобы выпустить плагин для конечных пользователей, необходимо выполнить следующие технические шаги:

### 1. Подпись и Нотаризация кода (macOS & Windows)
Без цифровой подписи операционные системы будут блокировать установку плагина.
- **macOS (Gatekeeper)**: Требуется аккаунт **Apple Developer** (стоимость $99/год). Бинарные файлы плагинов (`.vst3`, `.component`, `.aaxplugin`) должны быть подписаны сертификатом *Developer ID Application* и отправлены на нотаризацию в Apple через утилиту `xcrun notarytool`.
- **Windows (SmartScreen)**: Желательно подписать `.dll` / `.vst3` файлы сертификатом кода Windows (Code Signing Certificate), чтобы избежать предупреждения "Windows защитила ваш компьютер" при запуске установщика.

### 2. Подпись плагина для Pro Tools (PACE AAX Signing)
- Простая компиляция AAX-файла позволит запустить плагин только в специальной версии **Pro Tools Developer Build**.
- Для работы в коммерческой (retail) версии Pro Tools плагин должен быть подписан с помощью утилит **PACE Anti-Piracy** (PACE Sign Tool / `wraptool`).
- Для получения доступа к этим утилитам необходимо зарегистрироваться в программе **Avid Audio Development Partner** и подписать с ними соглашение. Подпись выполняется на этапе CI/CD с использованием физического USB-ключа iLok или облачной сессии iLok Eden.

### 3. Автоматизированное тестирование (Pluginval)
Перед релизом обязательно прогоните плагин через утилиту **Pluginval** (официальный инструмент валидации плагинов от JUCE):
- Скачайте `pluginval` с GitHub.
- Запустите тесты на утечки памяти, потокобезопасность (Thread Safety) и корректность обработки буферов в разных DAW.
- Проверьте плагин на уровнях строгости тестов до 10 включительно.

### 4. Создание установщиков (Installers)
Для удобства пользователей рекомендуется упаковать плагины в инсталляторы:
- **На macOS**: Создайте `.pkg`-установщик (например, с помощью бесплатной программы *Packages* или встроенных средств `pkgbuild`/`productbuild`). Инсталлятор должен автоматически копировать файлы плагина по путям:
  - VST3: `/Library/Audio/Plug-Ins/VST3/Atmocycle.vst3`
  - AU (Component): `/Library/Audio/Plug-Ins/Components/Atmocycle.component`
  - AAX: `/Library/Application Support/Avid/Audio/Plug-Ins/Atmocycle.aaxplugin`
- **На Windows**: Создайте инсталлятор с помощью **Inno Setup** или **NSIS**. Он должен устанавливать файлы по стандартным путям:
  - VST3: `C:\Program Files\Common Files\VST3\Atmocycle.vst3` (для 64-битной системы)
  - AAX: `C:\Program Files\Common Files\Avid\Audio\Plug-Ins\Atmocycle.aaxplugin`
