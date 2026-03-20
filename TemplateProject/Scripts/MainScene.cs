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
        Input.AddAxisVector2("move", Key.W, Key.S, Key.A, Key.D);
        new GameObject().AddComponent<Image>().gameObject.AddComponent<Test>();

        new GameObject().AddComponent<Image>().color = new(100);
        var t = new GameObject().AddComponent<Text>();
        t.text = "100HP";
        t.transform.position = new(-400, 200);
        t.color = new(200);
        t.isUI = true;
        t.fontSize = 6;
    }
}

public class Test : QEScript
{
    public override void Update()
    {
        transform.position += Input.GetAxisVector2("move");
        Camera.position = QMath.Lerp(Camera.position, transform.position, 0.02f);
    }
}