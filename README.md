
## 一. 版本说明
##### 程序可实现的功能：
1. **基本图元绘制**：支持直线、矩形、圆、椭圆、Bezier曲线、多边形、B样条曲线的绘制。
2. **实时绘制**：通过鼠标输入图形参数，实现实时绘制效果。
3. **菜单栏选择**：用户可通过点击上方菜单栏选择不同的绘制功能。
4. **状态栏**：在窗口底部设置状态栏，显示鼠标实时坐标，显示当前绘制状态。
5. **交点计算与突出**：
    - 计算两直线、直线与椭圆、直线与圆、两圆的交点。
    - 靠近交点时，用×绘出交点，并用红色实心突出交点。
6. **交点信息显示**：
    - 鼠标移动到交点附近时，在状态栏显示交点坐标。
    - 移动到圆或椭圆的边上时，显示圆心坐标，并采用同交点绘制的方式着重绘制交点。
7. **垂线绘制**：支持绘制某点到某直线的垂线，自动捕获垂足。
8. **鼠标交互**：
    - 当鼠标移动到图元上时，鼠标样式更改为十字。
    - 单击右键可删除当前选择的图元。
    - 选择曲线时，需将鼠标移动到曲线的端点上实现选择功能。
9. **清空画布**：通过点击菜单上的Clean(X)选项，实现清空画布功能。
10. **显示/隐藏栅格**：通过点击菜单上的ShowGrid(G)选项，实现显示或隐藏栅格。
11. **填充图形**：支持填充矩形和用多边形绘制的三角形。
12. **图形变换**：实现平移、旋转、缩放、裁剪功能。
##### 菜单图示：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/toolbar.png)
- 可通过**快捷键**操作（如按下Alt+L可进入绘制直线功能）。
- 如果是绘制基本图元，在状态栏上显示当前绘制的图元类型。
- Draw菜单下有子菜单项，提供绘制基本图元的方法。
- Transform菜单下有子菜单项，提供图形变换的方法。
## 二. 绘制图元操作说明
---
### 直线
1. 点击菜单上的"Draw->Line"或快捷键进入直线绘制状态，选择一种绘制方法，可使用Bresenham算法绘制直线，或调用D2D1库方法绘制直线；
2. 在窗口上用鼠标任意点一个点（左键）作为起点，拖动鼠标，可实现实时绘制直线，再次点击鼠标左键可实现这条直线的绘制；
3. 若完成一条直线的绘制，再次点击鼠标，可作为新的一条线段的起点，如此可在直线状态重复绘制直线。
 ![](https://gitee.com/tstwiki/melon/raw/master/images/line1.png)

 **绘制示例（用D2D1库方法绘制一条直线）：**
 ![](https://gitee.com/tstwiki/melon/raw/master/images/line2.png)

---
### 矩形
1. 点击菜单上的"Draw->Rectangle"或快捷键进入矩形绘制状态；
2. 在窗口用鼠标任意点击一个点（左键）作为起点，拖动鼠标，可实现实时绘制矩形，矩形的对角线为起点到鼠标位置，再次点击鼠标左键可实现矩形的绘制。默认绘制水平矩形，若要绘制倾斜椭圆，可在绘制了一个水平矩形后，使用“Rotate”功能进行旋转。；
3. 若完成一个矩形的绘制，再次点击鼠标，可作为新的矩形的起点，如此可在矩形状态重复绘制矩形。
 ![](https://gitee.com/tstwiki/melon/raw/master/images/rect1.png)
 <br>**绘制示例：**
 ![](https://gitee.com/tstwiki/melon/raw/master/images/rect2.png)
---
### 圆
1. 点击菜单上的"Draw->Circle"或快捷键进入圆绘制状态，选择一种绘制方法，可使用Bresenham算法绘制圆，或调用D2D1库方法绘制圆；
2. 在窗口上用鼠标任意点一个点（左键）作为起点（圆的圆心），拖动鼠标，可实现实时绘制圆，圆的半径为起点到鼠标的距离，再次点击鼠标可实现圆的绘制；
3. 同上，可重复绘制圆。
 ![](https://gitee.com/tstwiki/melon/raw/master/images/circle1.png)<br>
 **绘制示例（用Bresenham算法绘制一个圆）：**
 ![](https://gitee.com/tstwiki/melon/raw/master/images/circle2.png)
---
### 椭圆
1. 点击菜单上的"Draw->Ellipse"或快捷键进入椭圆绘制状态；
2. 在窗口上用鼠标任意点一个点（左键）作为起点（椭圆的圆心），拖动鼠标，可实现实时绘制椭圆，椭圆的圆心与鼠标构成一条线段，以这条线段为对角线的矩形的水平长度为椭圆的横半轴长，矩形的垂直长度为椭圆的竖半轴长。再次点击鼠标左键可实现椭圆的绘制；
3. 同上，可重复绘制椭圆。<br>
 ![](https://gitee.com/tstwiki/melon/raw/master/images/ellipse1.png)<br>
 **绘制示例**：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/ellipse2.png)
---
### Bezier(贝塞尔)曲线
1. 点击菜单上的"Draw->Bezier"或快捷键进入Bezier曲线绘制状态；
2. 在窗口连续左击四个点，分别作为曲线的起点、第一控制点、第二控制点、曲线的终点，在第三次左击鼠标后确定了第二控制点，可实时绘制Bezier曲线。一条贝塞尔曲线被绘制完成后，此时鼠标样式为十字，再次点击鼠标，可认为这条贝塞尔曲线被最终绘制完成，鼠标样式改为常规箭头。
3. 同上，可重复绘制Bezier曲线。
4. **注意：绘制完一条Bezier曲线的标志一定是鼠标从十字样式变为常规箭头样式，必须确认绘制完全才能进行其他操作。若操作不当，要恢复绘制，最简单的办法是重新启动应用。**
 
![](https://gitee.com/tstwiki/melon/raw/master/images/Bezier1.png)

**绘制示例：**
点击第一个点，此时绘制第一个点（用x和红色小圆点），鼠标样式为十字：
![](https://gitee.com/tstwiki/melon/raw/master/images/Bezier2.png)
点击第二个点：
![](https://gitee.com/tstwiki/melon/raw/master/images/Bezier3.png)
点击第三个点，此时屏幕上已经绘制了三个控制点，并以鼠标位置作为第四个控制点（临时的），实时绘制一条随鼠标移动而改变形状的Bezier曲线：
![](https://gitee.com/tstwiki/melon/raw/master/images/Bezier4.png)
点击第4个点，此时不绘制控制点，绘制一条确定的Bezier曲线，但鼠标样式仍为十字：
![](https://gitee.com/tstwiki/melon/raw/master/images/Bezier5.png)
点击第5个点，鼠标样式从十字变为常规箭头，认为这条Bezier曲线最终绘制完成。

---
### BLine(B样条曲线)

点击菜单上的"Draw->BLine"或快捷键进入BLine曲线绘制状态
参考Bezier曲线的绘制方法，在绘制BLine时会绘制出控制边<br>
**绘制示例（操作与Bezier曲线相同，也需要点击5个点，这里仅给出最终结果）：**
![](https://gitee.com/tstwiki/melon/raw/master/images/BLine.png)

***

### Polyline(多段线)
1. 点击菜单上的"Draw->Polyline"或快捷键进入Polyline绘制状态；
2. 可参考直线，多次左键点击，每个点都作为多段线的一个顶点，若绘制完成，点击菜单上的PolylineOK按钮，可实现这条多段线的绘制。
3. 同上，可重复绘制Polyline.<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline1.png)<br>
**绘制示例：**
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline2.png)
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline3.png)
绘制多义线时，如果鼠标靠近起点（多义线第一个点），突出绘制起点（用x和红色小圆点），可自动捕获，将鼠标移到起点，此时点击鼠标即可将当前直线段连接到起点，绘制成一个封闭区域，
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline4.png)
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline5.png)
若绘制完毕，可点击菜单栏的"PolylineOK"选项，或使用快捷键“Alt+O”结束当前多义线的绘制：
![](https://gitee.com/tstwiki/melon/raw/master/images/polyline6.png)

---
### 垂线
1. 点击菜单栏的“Perpendicular”选择绘制垂线功能时，状态进入"Line"，在窗口点击一个点，然后移动鼠标到任意一条线段，若移动到垂足大概位置附近，可着重绘制垂足，此时再点击鼠标可实现绘制垂线。
2. 要绘制多条垂线时，必须再次点击该菜单按钮来实现重复绘制。<br>
**绘制示例：**
这里有一条直线：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/perpendicular1.png)
 选择菜单栏的“Perpendicular”，点击直线外一个点，拖动鼠标，可实时绘制一条直线：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/perpendicular2.png)
 当鼠标移动该点到该直线大概的垂足位置（可目测）时，自动捕获垂足，并突出绘制（用x和红色小圆点）：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/perpendicular3.png)
 再次在该点附近点击鼠标左键（捕获垂足成功时），可绘制一条点到直线的垂线：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/perpendicular4.png)
 
---
### 填充
1. 首先在顶部菜单栏选中Fill，可以看到底部状态栏的Status更改为"Fill"；
2. 此时可将鼠标移至三角形或矩形，若鼠标样式由箭头更改为十字，说明当前选中图元；
3. 若选中图元，此时单击右键，可观察到选中的图元（目前仅支持三角形和矩形）被蓝色填充；
4. 在Fill状态下，若选中一个被填充的图元（鼠标移动至图形边缘），单击右键，可将该图元更改为未填充。<br>
**绘制示例：**
 这里有一个矩形：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/fill1.png)
 在顶部选择"Fill"，可看到底部状态栏“Status”从"Rectangle"变为"Fill"：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/fill2.png)
 此时将鼠标移动到矩形的边框上，鼠标样式改为十字，再单击右键，可看到该矩形被蓝色填充：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/fill3.png)
 若在Fill状态下，将鼠标移动到一个被填充的矩形的边框，可将该矩形设置为“未填充”：
 ![](https://gitee.com/tstwiki/melon/raw/master/images/fill4.png)
---

## 三. 图形变换
### 平移
1. 菜单选择"Translate":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/translate1.png)<br>
此时，底部状态栏提示状态为"Translate":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/translate2.png)
2. 鼠标左击选中图元，此时**松开**鼠标（**注意不是持续按下**），选中的图元可以跟随鼠标移动，再次点击鼠标可放下图元，认为已经平移完成。
3. 平移前：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/translate3.png)
   平移后：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/translate4.png)
   
***
### 旋转
1. 菜单选择"Rotate":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/rotate1.png)<br>
此时，底部状态栏提示状态为"Rotate":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/rotate2.png)
2. 鼠标左击选中图元，并以当前点为旋转中心（用红色实心小圆突出绘制），此时**松开**鼠标（**注意不是持续按下**），选中的图元可以跟随鼠标旋转，再次点击鼠标可放下图元，认为已经旋转完成。
3. 旋转前：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/rotate3.png)
   旋转中：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/rotate4.png)
   旋转后：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/rotate5.png)

***
### 缩放
1. 菜单选择"Zoom":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/zoom1.png)<br>
此时，底部状态栏提示状态为"Zoom":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/zoom2.png)
2. 鼠标左击选中图元，并以当前点为缩放比例尺起点，此时**松开**鼠标（**注意不是持续按下**），绘制一条从起点到以鼠标为终点的虚线段，缩放比例为这条虚线段的长度与200的比值。选中的图元可以跟随鼠标缩放，再次点击鼠标可放下图元，认为已经缩放完成。
3. 缩放前：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/zoom3.png)
   缩放中：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/zoom4.png)
   缩放后：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/zoom5.png)

---
### 裁剪
==**注意：裁剪的多边形必须是以逆时针方向绘制的封闭多边形**==

1. 菜单选择“Clip"<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/clip1.png)<br>
此时，底部状态栏提示状态为"Clip":<br>
![](https://gitee.com/tstwiki/melon/raw/master/images/clip2.png)
2. 鼠标点击一次，作为起点，松开，鼠标向右下方移动，此时绘制一个以起点为左上顶点，鼠标为右下顶点的裁剪矩形框，用红色虚线绘制。此时再点击鼠标左键，认为裁剪已经完成，删除原有图元。
3. 本程序可对直线和多段线进行真正意义上的裁剪，对于圆、椭圆和矩形，若完全在矩形框内，可进行裁剪。将裁剪得到的图形按比例放大到整个窗口。根据绘制窗口的长、宽与裁剪窗口的长、宽，确定适合的比例放大到整个绘制窗口（如，绘制窗口的x/y小于裁剪窗口的x/y，则缩放比例为绘制窗口的x除以裁剪窗口的x，并将放大后的裁剪窗口左上顶点移动到绘制窗口的左上顶点，也即原点(0,0)).
4. 裁剪前：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip3.png)
   裁剪中：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip4.png)
   裁剪后：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip5.png)
   **裁剪多边形示例：**
   这里有一个五边形：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip6.png)
   用一个矩形裁剪该多边形：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip7.png)
   将裁剪后的结果按比例放大到整个画布：
   ![](https://gitee.com/tstwiki/melon/raw/master/images/clip8.png)
   
   ---
   
## 四. 运行界面

![](https://gitee.com/tstwiki/melon/raw/master/images/run1.png)
![](https://gitee.com/tstwiki/melon/raw/master/images/run2.png)