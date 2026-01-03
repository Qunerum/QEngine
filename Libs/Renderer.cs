using System;
using System.Globalization;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Threading;
using Avalonia.Media;

using QEngine;
using QEngine.Input;
using QEngine.Mathematics;

using Image = QEngine.GUI.Image;
using Text = QEngine.GUI.Text;
using Button = QEngine.GUI.Button;
using Slider = QEngine.GUI.Slider;

public sealed class GameWindow : Window
{
    readonly GameView _view;

    public GameWindow()
    {
        Width = Game.size.x;
        Height = Game.size.y;
        Title = Game.title;

        PointerMoved += OnPointerMoved;
        PointerPressed += OnPointerPressed;
        PointerReleased += OnPointerReleased;
        
        KeyDown += OnKeyDown;
        KeyUp += OnKeyUp;
        
        _view = new GameView();
        Content = _view;

        StartGameLoop();
    }
    void UpdateMP(Point p) => Input.mousePosition = new((int)p.X - (int)(Width / 2), -((int)p.Y - (int)(Height / 2)));
    void OnPointerMoved(object? sender, PointerEventArgs? e)
    {
        var p = e.GetPosition(_view); UpdateMP(p);
    }
    void OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        var p = e.GetPosition(_view); UpdateMP(p);
        Input.mouseLeft = true;
    }
    void OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        var p = e.GetPosition(_view); UpdateMP(p);
        Input.mouseLeft = false;
    }

    protected override void OnResized(WindowResizedEventArgs e)
    {
        Game.size = new((int)e.ClientSize.Width, (int)e.ClientSize.Height);
        base.OnResized(e);
    }

    void OnKeyDown(object? sender, KeyEventArgs e)
    {
        if (KeyMapper.TryMap(e.Key, out var key))
            Input.SetKeyDown(key);
    }
    void OnKeyUp(object? sender, KeyEventArgs e)
    {
        if (KeyMapper.TryMap(e.Key, out var key))
            Input.SetKeyUp(key);
    }
    void StartGameLoop()
    {
        var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(16) };

        timer.Tick += (_, _) =>
        {
            _view.Update();
            _view.InvalidateVisual();
        };

        timer.Start();
    }
}

public sealed class GameView : Control
{
    public void Update() { SceneManager.actualScene?.Update(); }
    public override void Render(DrawingContext ctx)
    {
        ctx.FillRectangle(Brushes.Black, Bounds);
        if (SceneManager.actualScene == null)
            return;
        base.Render(ctx);
        
        for (int i = 0; i < SceneManager.actualScene.getObjects().Count; i++)
        {
            GameObject obj = SceneManager.actualScene.getObjects()[i];
            Vector2 pos = new(obj.transform.position.x + Game.size.x / 2, -obj.transform.position.y + Game.size.y / 2);
            // = = = = = IMAGE = = = = = 
            if (obj.TryGetComponent(out Image img))
            { 
                if (img.sprite != null)
                {
                    ctx.DrawImage(
                        img.sprite.Bitmap,
                        new Rect(0, 0, img.sprite.Width, img.sprite.Height), // SOURCE
                        new Rect(
                            pos.x - img.sprite.Width / 2 * obj.transform.scale.x,
                            pos.y - img.sprite.Height / 2 * obj.transform.scale.y,
                            img.sprite.Width * obj.transform.scale.x,
                            img.sprite.Height * obj.transform.scale.y
                        )
                    );
                    //ctx.FillRectangle(color, new Rect(obj.transform.position.x, obj.transform.position.y, img.sprite.Width, img.sprite.Height));
                }
                else { ctx.FillRectangle(img.color._clr,
                    new Rect(pos.x - img.size.x / 2, pos.y - img.size.y / 2, img.size.x, img.size.y)); }
            } 
            // = = = = = TEXT = = = = = 
            if (obj.TryGetComponent(out Text text))
            { 
                ctx.DrawText(new(text.text, CultureInfo.InvariantCulture, FlowDirection.LeftToRight, new Typeface("Arial"),
                        text.fontSize, text.color._clr), new Point(pos.x, pos.y));
            } 
            // = = = = = BUTTON = = = = = 
            if (obj.TryGetComponent(out Button button))
            {
                Rect r = new Rect(pos.x - button.size.x / 2 * button.transform.scale.x,
                    pos.y - button.size.y / 2 * button.transform.scale.y, button.size.x, button.size.y);
                Vector2Int mp = Input.mousePosition;
                bool isOn = isOnUI(button, button.size.x, button.size.y);
                if (Input.mouseLeft && isOn && !button.isOn) { button.isOn = true; button.onClick?.Invoke(); }
                if (!Input.mouseLeft && isOn && button.isOn) { button.isOn = false; }
                
                if (isOn && !button.isEnter) { button.isEnter = true; button.onPointerEnter?.Invoke(); }
                if (!isOn && button.isEnter) { button.isEnter = false; button.onPointerExit?.Invoke(); }

                ctx.FillRectangle(button.color._clr, r);
            }
            // = = = = = SLIDER = = = = =  
            if (obj.TryGetComponent(out Slider slider))
            {
                Vector2Int mp = Input.mousePosition;
                Rect r = new(pos.x - slider.size.x / 2 * slider.transform.scale.x,
                    pos.y - slider.size.y / 2 * slider.transform.scale.y, slider.size.x, slider.size.y);
                ctx.FillRectangle(slider.backgroundColor._clr, r);
                // Fill 
                float fillPer = slider.GetValue() / (slider.GetMax() - slider.GetMin()) * slider.size.x;
                ctx.FillRectangle(slider.fillColor._clr, new(r.X, r.Y, fillPer, slider.size.y));
                // Handle
                double hw = 10;
                ctx.FillRectangle(slider.handleColor._clr, new(r.X + fillPer - hw / 2, r.Y - 5, hw, slider.size.y + 10));

                double mouseMin = slider.transform.position.x - slider.size.x / 2 * slider.transform.scale.x;
                double mouseMax = slider.transform.position.x + slider.size.x / 2 * slider.transform.scale.x;
                float valMouse = QMath.Round(QMath.Remap(mp.x, (float)mouseMin, (float)mouseMax, slider.GetMin(), slider.GetMax()), slider.valueDecimals);
                
                if (isOnUI(slider, slider.size.x, slider.size.y) && Input.mouseLeft && !slider.isHolding) { slider.isHolding = true; }
                if (!Input.mouseLeft && slider.isHolding) { slider.isHolding = false; }
                
                if (slider.isHolding) { slider.SetValue(valMouse); }
            }
        }
    }

    public bool isOnUI(Component component, float sizeX, float sizeY)
    {
        Vector2Int mp = Input.mousePosition;
        float left = -sizeX / 2 + component.transform.position.x;
        float right = sizeX / 2 + component.transform.position.x;
        float up = sizeY / 2 + component.transform.position.y;
        float bottom = -sizeY / 2 + component.transform.position.y;
        return 
            mp.x > left && mp.x < right &&
            mp.y < up && mp.y > bottom;  
    }
}
public class App : Application
{
    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            SceneManager.AddScene("Main", new MainScene());
            SceneManager.GoToScene("Main");
            
            desktop.MainWindow = new GameWindow();
        }
        base.OnFrameworkInitializationCompleted();
    }
}