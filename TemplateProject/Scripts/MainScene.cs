using System.Collections.Generic;
using QEngine;
using QEngine.GUI;
using QEngine.Input;
using QEngine.Mathematics;
using QEngine.Text;

public class MainScene : QEScene
{
    public override void Init()
    {
        Game.title = "QEngine Project 0.4.0";
        Game.SetResolution(800, 600);
    }
}