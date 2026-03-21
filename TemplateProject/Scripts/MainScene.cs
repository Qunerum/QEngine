using System.Collections.Generic;
using QEngine;
using QEngine.Dev.Renderer;
using QEngine.GUI;
using QEngine.Input;
using QEngine.Mathematics;
using QEngine.Text;

public class MainScene : QEScene
{
    public override void Init()
    {
        Game.title = "QEngine Project 0.5.0";
        Game.SetResolution(800, 600);
        
        Input.AddAxisVector2("cam", Key.Left, Key.Right, Key.Down, Key.Up);
        
        Input.AddAxisVector2("moveXZ", Key.W, Key.S, Key.A, Key.D);
        Input.AddAxisInt("moveY", Key.Q, Key.E);
        new GameObject().AddComponent<program>();
        Camera.lightPosition = new(0, 5, 0);
        Camera.position = new Vector3(0, 5, -2);
        Camera.rotation = new Vector3(0, 0 , 0);
    }
}

public class program : QEScript
{
    public override void Update()
    {
        Vector2 moveXZ = Input.GetAxisVector2("moveXZ");
        float moveY = Input.GetAxisInt("moveY");
        Camera.lightPosition += new Vector3(-moveXZ.x, moveY, moveXZ.y) / 25f;

        Camera.rotation += Input.GetAxisVector2("cam").toVector3() / 2f;
        QRenderer.DrawGeometry(new(),
            [
                new(-1, 0, 1), 
                new(1, 0, 1), 
                new(1, 0, -1), 
                new(-1, 0, -1), 
                new(0, 1, 0)], 
            [
                0, 2, 1, 0, 3, 2,

                0, 4, 3,
                3, 4, 2,
                2, 4, 1,
                1, 4, 0 
            ], new Color(0, 150, 0).to01());
        QRenderer.DrawCube(Camera.lightPosition, new(0.25f, 0.25f, 0.25f), new Color(200, 200, 0).to01());
    }
}