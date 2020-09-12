#pragma once

#include <Shlobj.h>
#include <concurrent_vector.h>
#include <pathcch.h>
//#include <objidl.h>
#include <gdiplus.h>
#include <Windowsx.h>
#include <Uxtheme.h>
#include "resource.h"
#include "DialogService.h"
#include "DelaunayInitialization.h"
#include "DelaunayTriangulation.h"
#include "DelaunayWriteToFile.h"
#include "DelaunayDeinitializition.h"

#pragma comment(lib, "pathcch")

#pragma comment(lib,"Uxtheme")

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

using namespace concurrency;

// Тип BitmapPtr позволяет указателю на объект Bitmap
// совместно использоваться несколькими компонентами.
// Предназначен для рисования в нём триангуляционной сетки
// (которое выполняется в фоновом потоке).
// Объект  Bitmap удаляется,  когда на него больше не 
// ссылается ни один компонент.
typedef std::shared_ptr<Gdiplus::Bitmap> BitmapPtr;
// Группа задач для перерисовки триангуляционной сетки
// всвязи с изменением размера экрана.
concurrency::task_group redrawingTasks;
// Буфер для выполненого изображения триангуляционной сетки.
// Через него изображение триангуляционной сетки передаётся
// фонового потока, в котором оно создавалось, в поток UI
// для показа в клиентской области окна приложения.
concurrency::unbounded_buffer<BitmapPtr> m_TriMeshImages;

