using QEngine;
using QEngine.GUI;

public class MainScene : QEScene
{
    public override void Init()
    {
        Game.title = $"QEngine ";
        Game.SetResolution(800, 600);
        
        var img = new GameObject().AddComponent<Image>();
        img.color = new(100);
    }
}
