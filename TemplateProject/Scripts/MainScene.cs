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
        
        Input.AddAxisVector2("move", Key.Up, Key.Down, Key.Left, Key.Right);
        
        Input.AddAxisVector2("moveXZ", Key.D, Key.A, Key.W, Key.S);
        Input.AddAxisInt("moveY", Key.Q, Key.E);
        new GameObject().AddComponent<program>();
        Camera.lightPosition = new(2, 0, 2);
        Camera.position = new Vector3(0, 0, -5);
        Camera.rotation = new Vector3(0, 0 , 0);
        Logger.Log("Test Log!");
        Logger.Warning("Test Warning!");
        Logger.Error("Test Error!");
    }
}

public class program : QEScript
{
    public override void Update()
    {
        Vector2 moveXZ = Input.GetAxisVector2("moveXZ");
        Camera.rotation -= new Vector3(moveXZ.x, moveXZ.y, 0);

        if (Input.mouseLeft)
        {
            Camera.position += new Transform(Camera.position, Camera.rotation).forward / 100;
        }
        
        var l = Input.GetAxisVector2("move");
        Camera.lightPosition += new Vector3(-l.x, 0, l.y) / 20;
        
        QRenderer.DrawSphere(Camera.lightPosition, new(), 0.2f, 6, 6, new Color(200, 200, 0).to01());
        
        QRenderer.DrawCapsule(new(), new(), 0.5f, 2, 8, Color.White.to01());
        QRenderer.DrawBox(new(1, 0, 0), new(), new(1, 2, 1), Color.Blue.to01());
    }
}