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
        
        
        Input.AddAxisVector2("moveXZ", Key.W, Key.S, Key.A, Key.D);
        Input.AddAxisInt("moveY", Key.Q, Key.E);
        new GameObject().AddComponent<program>();
        Camera.lightPosition = new(2, 2, 0);
        Camera.position = new Vector3(0, 0.6f, -10);
        Camera.rotation = new Vector3(0, 0 , 0);
    }
}

public class program : QEScript
{
    Vector3 rot;
    public override void Update()
    {
        Vector2 moveXZ = Input.GetAxisVector2("moveXZ");
        float moveY = Input.GetAxisInt("moveY");
        rot += new Vector3(moveXZ.x, moveXZ.y, moveY);
        QRenderer.DrawCube(Camera.lightPosition, new(), new(0.25f, 0.25f, 0.25f), new Color(200, 200, 0).to01());
        
        QRenderer.DrawGeometry(new(), rot,
            [
                new(-1, 0, 1), 
                new(1, 0, 1), 
                new(1, 0, -1), 
                new(-1, 0, -1), 
                new(0, 0.5f, 0)], 
            [
                0, 2, 1, 0, 3, 2,

                0, 4, 3,
                3, 4, 2,
                2, 4, 1,
                1, 4, 0 
            ], new Color(100).to01());
    }
}