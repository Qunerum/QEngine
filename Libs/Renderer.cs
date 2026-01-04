using System;
using System.Globalization;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Threading;
using Avalonia.Media;

using QEngine;
using QEngine.GUI;
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
                }
                else { ctx.FillRectangle(img.color._clr,
                    CenterRect(pos, img.size, img.transform.scale)); }
            } 
            // = = = = = TEXT = = = = = 
            if (obj.TryGetComponent(out Text text))
            { 
                var formatted = new FormattedText(
                    text.text,
                    CultureInfo.InvariantCulture,
                    FlowDirection.LeftToRight,
                    new Typeface("Arial"),
                    text.fontSize,
                    text.color._clr
                );
                var x = pos.x - formatted.Width / 2;
                var y = pos.y - formatted.Height / 2;
                ctx.DrawText(formatted, new Point(x, y));
            } 
            // = = = = = BUTTON = = = = = 
            if (obj.TryGetComponent(out Button button))
            {
                Rect r = CenterRect(pos, button.size, button.transform.scale);
                Vector2Int mp = Input.mousePosition;
                bool isOn = isOnUI(button.transform.position, button.size, button.transform.scale);
                if (Input.mouseLeft && isOn && !button.isOn) { button.isOn = true; button.onClick?.Invoke(); }
                if (!Input.mouseLeft && button.isOn) { button.isOn = false; }
                
                if (isOn && !button.isEnter) { button.isEnter = true; button.onPointerEnter?.Invoke(); }
                if (!isOn && button.isEnter) { button.isEnter = false; button.onPointerExit?.Invoke(); }

                ctx.FillRectangle(button.color._clr, r);
            }
            // = = = = = SLIDER = = = = =  
            if (obj.TryGetComponent(out Slider slider))
            {
                Vector2Int mp = Input.mousePosition;
                Rect r = CenterRect(pos, slider.size, slider.transform.scale);
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
                
                if (isOnUI(slider.transform.position, slider.size, slider.transform.scale) && Input.mouseLeft && !slider.isHolding) { slider.isHolding = true; }
                if (!Input.mouseLeft && slider.isHolding) { slider.isHolding = false; }
                
                if (slider.isHolding) { slider.SetValue(valMouse); }
            }
            // < = = = = = > DROPDOWN < = = = = = >
            if (obj.TryGetComponent(out Dropdown dropdown))
            {
                dropdown.option = Math.Clamp(dropdown.option, 0, dropdown.options.Count - 1);
                Rect r = CenterRect(pos, dropdown.size, dropdown.transform.scale);
                bool isOn = isOnUI(dropdown.transform.position, dropdown.size, dropdown.transform.scale);
                Console.WriteLine(Input.mousePosition);
                Console.WriteLine(isOn);
                Console.WriteLine("===");
                ctx.FillRectangle(dropdown.color._clr, r);
                
                var formatted = new FormattedText(
                    dropdown.options[dropdown.option],
                    CultureInfo.InvariantCulture,
                    FlowDirection.LeftToRight,
                    new Typeface("Arial"),
                    dropdown.labelFontSize,
                    dropdown.labelFontColor._clr
                );
                var x = pos.x - formatted.Width / 2;
                var y = pos.y - formatted.Height / 2;
                ctx.DrawText(formatted, new Point(x, y));
                
                if (Input.mouseLeft && isOn && !dropdown.isOn) { dropdown.isOn = true; dropdown.isOpened = !dropdown.isOpened; }
                if (!Input.mouseLeft && isOn && dropdown.isOn) { dropdown.isOn = false; }

                if (dropdown.isOpened)
                {
                    for (int o = 1; o <= dropdown.options.Count; o++)
                    {
                        float down = dropdown.optionsDistance + (dropdown.optionSize.y + dropdown.optionsDistance) * o;
                        Vector2 oCenterWorld = new(dropdown.transform.position.x, dropdown.transform.position.y - down);
                        Vector2 oCenterScreen = new(pos.x, pos.y + down);
                        Rect or = CenterRect(oCenterScreen, dropdown.optionSize, dropdown.transform.scale);
                        bool oisOn = isOnUI(oCenterWorld, dropdown.optionSize, dropdown.transform.scale);
                        if (Input.mouseLeft && oisOn) { dropdown.isOpened = false; dropdown.option = o - 1; }
                        
                        var oFormatted = new FormattedText(
                            dropdown.options[o-1],
                            CultureInfo.InvariantCulture,
                            FlowDirection.LeftToRight,
                            new Typeface("Arial"),
                            dropdown.optionFontSize,
                            dropdown.optionFontColor._clr
                        );
                        ctx.FillRectangle(dropdown.optionColor._clr, or);
                        
                        var ox = oCenterScreen.x - oFormatted.Width / 2; var oy = oCenterScreen.y - oFormatted.Height / 2;
                        ctx.DrawText(oFormatted, new Point(ox, oy));
                    }
                }
            }
        }
    }
    static Rect CenterRect(Vector2 center, Vector2 size, Vector2 scale)
    {
        float w = size.x * scale.x;
        float h = size.y * scale.y;
        return new Rect(center.x - w / 2, center.y - h / 2, w, h);
    }

    public bool isOnUI(Vector2 center, Vector2 size, Vector2 scale)
    {
        Vector2Int mp = Input.mousePosition;
        float halfW = size.x * scale.x / 2;
        float halfH = size.y * scale.y / 2;
        return
            mp.x >= center.x - halfW &&
            mp.x <= center.x + halfW &&
            mp.y >= center.y - halfH &&
            mp.y <= center.y + halfH;
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