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
using Color = QEngine.GUI.Color;
using Slider = QEngine.GUI.Slider;

public sealed class GameWindow : Window
{
    readonly GameView _view;

    public GameWindow()
    {
        Width = Game.size.x;
        Height = Game.size.y;
        Title = Game.title;

        TextInput += Input.SetTextInput;
        
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
            Input.Update();
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
        ctx.FillRectangle(Game.background._clr, Bounds);
        if (SceneManager.actualScene == null)
            return;
        base.Render(ctx);
        
        for (int i = 0; i < SceneManager.actualScene.GetObjects().Count; i++)
        {
            GameObject obj = SceneManager.actualScene.GetObjects()[i];
            Vector2 pos = new(obj.transform.position.x - SceneManager.actualScene.cameraPosition.x + Game.size.x / 2, -obj.transform.position.y + SceneManager.actualScene.cameraPosition.y + Game.size.y / 2);
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
            // = = = = = SHAPE 2D = = = = =
            if (obj.TryGetComponent(out Shape2D shape))
            {
                if (shape.GetVertices().Count < 2)
                    return;
                var geo = new StreamGeometry();
                using (var ctxGeo = geo.Open())
                {
                    var t = obj.transform;

                    Point Transform(Vector2 v)
                    {
                        float x = v.x * t.scale.x + pos.x;
                        float y = -v.y * t.scale.y + pos.y;
                        return new Point(x, y);
                    }

                    ctxGeo.BeginFigure(Transform(shape.GetVertices()[0]), true);

                    for (int j = 1; j < shape.GetVertices().Count; j++)
                        ctxGeo.LineTo(Transform(shape.GetVertices()[j]));
                    ctxGeo.EndFigure(true);
                }
                ctx.DrawGeometry(shape.color._clr, null, geo);
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
                ctx.FillRectangle(button.color._clr, r);
            }
            // = = = = = SLIDER = = = = =  
            if (obj.TryGetComponent(out Slider slider))
            {
                Rect r = CenterRect(pos, slider.size, slider.transform.scale);
                ctx.FillRectangle(slider.backgroundColor._clr, r);
                // Fill 
                float fillPer = slider.GetValue() / (slider.GetMax() - slider.GetMin()) * slider.size.x;
                ctx.FillRectangle(slider.fillColor._clr, new(r.X, r.Y, fillPer, slider.size.y));
                // Handle
                double hw = 10;
                ctx.FillRectangle(slider.handleColor._clr, new(r.X + fillPer - hw / 2, r.Y - 5, hw, slider.size.y + 10));
            }
            // < = = = = = > DROPDOWN < = = = = = > 
            if (obj.TryGetComponent(out Dropdown dropdown))
            {
                dropdown.option = Math.Clamp(dropdown.option, 0, dropdown.options.Count - 1);
                Rect r = CenterRect(pos, dropdown.size, dropdown.transform.scale);
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
            // < = = = = = > INPUT FIELD < = = = = = > 
            if (obj.TryGetComponent(out InputField inputField))
            {
                Rect r = CenterRect(pos, inputField.size, inputField.transform.scale);
                ctx.FillRectangle(inputField.color._clr, r);
                
                string txt = inputField.text.Length > 0 ? inputField.text : inputField.labelText;
                Color clr = inputField.text.Length > 0 ? inputField.textColor : inputField.labelTextColor; 
                float fSize = inputField.text.Length > 0 ? inputField.textFontSize : inputField.labelTextFontSize;
                var formatted = new FormattedText(txt, CultureInfo.InvariantCulture, FlowDirection.LeftToRight, new Typeface("Arial"), fSize, clr._clr);
                var x = pos.x - formatted.Width / 2; var y = pos.y - formatted.Height / 2;
                ctx.DrawText(formatted, new Point(x, y));
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