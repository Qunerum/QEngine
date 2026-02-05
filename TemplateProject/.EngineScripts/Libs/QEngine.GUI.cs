using System;
using System.Collections.Generic;

using Veldrid;

using QEngine.Dev.Renderer;
using QEngine.Input;
using QEngine.Mathematics;
using QEngine.Text;

using Key = QEngine.Input.Key;

namespace QEngine.GUI
{
    public static class GUI
    {
        public static ushort[] quad = { 0, 1, 2, 0, 2, 3 };
        public static bool isOnUI(Vector2 center, Vector2 size)
        {
            Vector2 mp = new(Cursor.position.x, Cursor.position.y);
            float halfW = size.x / 2;
            float halfH = size.y / 2;
            return
                mp.x >= center.x - halfW &&
                mp.x <= center.x + halfW &&
                mp.y >= center.y - halfH &&
                mp.y <= center.y + halfH;
        }
    }

    #region Variables
    public struct Color
    {
        public byte r, g, b, a;
        public Color(byte rgb, byte a = 255) { r = rgb; g = rgb; b = rgb; this.a = a; }
        public Color(byte r, byte g, byte b, byte a = 255) { this.r = r; this.g = g; this.b = b; this.a = a; }
        
        public Vector4 to01() => new Vector4(r, g, b, a) / 255f;
        public RgbaFloat toRgba() => new(r/255f, g/255f, b/255f, a/255f);
        
        public static Color Red => new(255, 0, 0);
        public static Color Green => new(0, 255, 0);
        public static Color Blue => new(0, 0, 255);
        public static Color White => new(255);
        public static Color Black => new(0);
    }

    public class Sprite
    {
        public int Width, Height;
        public Vector4 uv;
        public byte[] pixels;
        public Sprite(int Width, int Height, byte[] pxs)
        { this.Width = Width; this.Height = Height; pixels = pxs; }
    }
    #endregion
    
    #region Components
    public class Image : Component, IRenderable
    {
        public Vector2 size = new(100, 100);
        public Sprite sprite;
        public Color color = Color.White;

        public void Draw()
        {
            if (sprite != null)
            { QRenderer.DrawSprite(sprite, sprite.uv, transform.position, size, color.to01()); } else
            {
                QRenderer.DrawShape(transform.position,
                    new Vector2[] {
                        new(-size.x / 2, -size.y / 2),
                        new(size.x / 2, -size.y / 2),
                        new(size.x / 2, size.y / 2),
                        new(-size.x / 2, size.y / 2)
                    }, GUI.quad, color.to01());
            }
        }
    }
    public class Shape2D : Component, IRenderable
    {
        List<Vector2> vertices = new() { new(0, 83.2f), new(-100, -80), new(100, -80) };
        List<ushort> indices = new() { 0, 2, 1 };
        public Color color = Color.White;

        public void Clear() { vertices.Clear(); indices.Clear(); }

        public List<Vector2> GetVertices() => vertices;
        public void AddVertex(Vector2 vertex) => vertices.Add(vertex);
        public void SetVertices(List<Vector2> vertices) => this.vertices = vertices;
        public void RemoveVertex(int index) => vertices.RemoveAt(Math.Clamp(index, 0, vertices.Count - 1));
        public List<ushort> GetIndices() => indices;
        public void AddIndice(ushort indice) => indices.Add(indice);
        public void SetIndices(List<ushort> indices) => this.indices = indices;
        public void RemoveIndice(int index) => indices.RemoveAt(Math.Clamp(index, 0, vertices.Count - 1));
        public void Draw() => QRenderer.DrawShape(transform.position,vertices.ToArray(), indices.ToArray(), color.to01());
    }
    public class Button : Component, IRenderable
    {
        public Vector2 size = new(100, 100);
        public Color color = new(255);
        
        public Action onClick;
        public Action onPointerEnter;
        public Action onPointerExit;
        
        bool isOn = false;
        bool isEnter = false;

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; onClick?.Invoke(); }
            if (!Input.Input.mouseLeft && isOn) { isOn = false; }
                
            if (_isOn && !isEnter) { isEnter = true; onPointerEnter?.Invoke(); }
            if (!_isOn && isEnter) { isEnter = false; onPointerExit?.Invoke(); }
        }
        
        public void Draw()
        {
            QRenderer.DrawShape(transform.position,
                new Vector2[] {
                    new(-size.x / 2, -size.y / 2),
                    new(size.x / 2, -size.y / 2),
                    new(size.x / 2, size.y / 2),
                    new(-size.x / 2, size.y / 2)
                }, GUI.quad, color.to01());
        }
    }
    public class Slider : Component, IRenderable
    {
        public Vector2 size = new(200, 20);
        public Color backgroundColor = new(255, 100);
        public Color fillColor = new(160);
        public Color handleColor = new(200);
        
        public int valueDecimals = 2;
        
        public Action<float> onValueChanged;

        public bool isInteractable = true, hideHandle = false;

        bool isHolding = false;
        
        float minRange = 0;
        float maxRange = 10;
        float value = 5;
        
        public float GetMin() => minRange;
        public float GetMax() => maxRange;
        public float GetValue() => value;

        public float SetMin(float min) => minRange = min;
        public float SetMax(float max) => maxRange = max;
        public void SetMinAndMax(Vector2 minMax) { minRange = minMax.x; maxRange = minMax.y; }
        public void SetMinAndMax(float min, float max) { minRange = min; maxRange = max; }
        public float SetValue(float value) => this.value = Math.Clamp(value, minRange, maxRange);
        float oldValue = 0;
        public override void Update()
        {
            if (!isInteractable) return; 
            Vector2Int mp = Cursor.position;
            double mouseMin = transform.position.x - size.x / 2;
            double mouseMax = transform.position.x + size.x / 2;
            float valMouse = QMath.Round(QMath.Remap(mp.x, (float)mouseMin, (float)mouseMax, minRange, maxRange), valueDecimals);
            if (GUI.isOnUI(transform.position, size) && Input.Input.mouseLeft && !isHolding) { isHolding = true; }
            if (!Input.Input.mouseLeft && isHolding) { isHolding = false; }
            if (isHolding) { SetValue(valMouse); if (value != oldValue) 
                { oldValue = value; onValueChanged?.Invoke(value); } }
        }
        public void Draw()
        {
            QRenderer.DrawShape(transform.position,
                new Vector2[] {
                    new(-size.x / 2, -size.y / 2),
                    new(size.x / 2, -size.y / 2),
                    new(size.x / 2, size.y / 2),
                    new(-size.x / 2, size.y / 2)
                }, GUI.quad, backgroundColor.to01());
            float newX = QMath.Remap(value, minRange, maxRange, -size.x / 2, size.x / 2);
            QRenderer.DrawShape(transform.position,
                new Vector2[] {
                    new(-size.x / 2, -size.y / 2),
                    new(newX, -size.y / 2),
                    new(newX, size.y / 2),
                    new(-size.x / 2, size.y / 2)
                }, GUI.quad, fillColor.to01());
            if (!hideHandle)
                QRenderer.DrawShape(transform.position,
                    new Vector2[]
                    {
                        new(newX - 5, -size.y / 2 - 5),
                        new(newX + 5, -size.y / 2 - 5),
                        new(newX + 5, size.y / 2 + 5),
                        new(newX - 5, size.y / 2 + 5)
                    }, GUI.quad, handleColor.to01());
            
        }
    }
    public class Dropdown : Component, IRenderable
    {
        public Vector2 size = new(200, 30);
        public Color color = new(255);

        public Font font = Assets.GetFont("Default");
        public float fontSpace = 8;
        
        public int labelFontSize = 6;
        public Color labelFontColor = new(0);

        public Vector2 optionSize = new(180, 20);
        public Color optionColor = new(200);
        public int optionFontSize = 4;
        public Color optionFontColor = new(0);

        public float optionsDistance = 5;

        public int option = 0;

        public List<string> options = new() { "Option A", "Option B", "Option C" };

        bool isOn = false;
        bool isOn_ = false;
        public bool isOpened = false;

        public void Clear() => options.Clear();
        public void AddOption(string option) => options.Add(option);
        public void RemoveOption(int index) => options.RemoveAt(Math.Clamp(index, 0, options.Count - 1));

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size);
            if (Input.Input.mouseLeft && _isOn && !isOn) { isOn = true; isOpened = !isOpened; }
            if (!Input.Input.mouseLeft && _isOn && isOn) { isOn = false; }

            if (isOpened)
            {
                for (int i = 0; i < options.Count; i++)
                {
                    Vector2 pos = transform.position - new Vector2(0, optionsDistance) - new Vector2(0, optionSize.y + optionsDistance) * (i + 1);
                    bool _isOn_ = GUI.isOnUI(pos, optionSize);
                    if (_isOn_ && !isOn_ && Input.Input.mouseLeft) { option = i; isOpened = false; }
                }
            }
        }
        
        public void Draw()
        {
            QRenderer.DrawShape(transform.position,
                new Vector2[] {
                    new(-size.x / 2, -size.y / 2),
                    new(size.x / 2, -size.y / 2),
                    new(size.x / 2, size.y / 2),
                    new(-size.x / 2, size.y / 2)
                }, GUI.quad, color.to01());
            QRenderer.DrawText(font, fontSpace, options[option], transform.position - new Vector2(size.x / 2 - 5, size.y / 2 - labelFontSize / 2f), labelFontSize, labelFontColor.to01());
            if (isOpened)
            {
                for (int i = 0; i < options.Count; i++)
                {
                    Vector2 pos = transform.position - new Vector2(0, optionsDistance) - new Vector2(0, optionSize.y + optionsDistance) * (i + 1);
                    QRenderer.DrawShape(pos, new Vector2[] {
                        new(-optionSize.x / 2, -optionSize.y / 2),
                        new(optionSize.x / 2, -optionSize.y / 2),
                        new(optionSize.x / 2, optionSize.y / 2),
                        new(-optionSize.x / 2, optionSize.y / 2)
                    }, GUI.quad, color.to01());
                    QRenderer.DrawText(font, fontSpace, options[i], pos - new Vector2(optionSize.x / 2 - 5, optionSize.y / 2 - optionFontSize / 2f), optionFontSize, optionFontColor.to01());
                }
            }
        }
    }
    public class InputField : Component, IRenderable
    {
        public Vector2 size = new(200, 30);
        public Color color = Color.White;
        
        public float fontSpace = 8;
        public Font font = Assets.GetFont("Default");
        public string text = "";
        public Color textColor = new(0);
        public int textFontSize = 6;
        
        public string labelText = "Enter text...";
        public Color labelTextColor = new(0, 150);
        public int labelFontSize = 6;

        bool isTexting = false;
        bool isBack = false;

        public override void Update()
        {
            bool _isOn = GUI.isOnUI(transform.position, size);
            if (Input.Input.mouseLeft && _isOn && !isTexting) { isTexting = true; Input.Input.data = string.Empty; }
            if (Input.Input.mouseLeft && !_isOn && isTexting) { isTexting = false; }
            
            if (isTexting && Input.Input.Read(Key.Backspace) && !isBack && text.Length > 0) { text = text[..^1]; isBack = true; }
            if (!Input.Input.Read(Key.Backspace)) { isBack = false; }
            if (isTexting && Input.Input.data.Length > 0) { text += Input.Input.data; Input.Input.data = string.Empty; }
        }
        
        public void Draw()
        {
            QRenderer.DrawShape(transform.position,
                new Vector2[] {
                    new(-size.x / 2, -size.y / 2),
                    new(size.x / 2, -size.y / 2),
                    new(size.x / 2, size.y / 2),
                    new(-size.x / 2, size.y / 2)
                }, GUI.quad, color.to01());
            if (text.Length > 0) QRenderer.DrawText(font, fontSpace, text, transform.position - new Vector2(size.x / 2 - 5, size.y / 2 - textFontSize / 2f), textFontSize, textColor.to01());
            else QRenderer.DrawText(font, fontSpace, labelText, transform.position - new Vector2(size.x / 2 - 5, size.y / 2 - labelFontSize / 2f), labelFontSize, labelTextColor.to01());
        }
    }

    #region Particle System
    public class ParticleData
    {
        public Vector2 position = new();
        
        public Color startColor = Color.White;
        public Color color = Color.White;
        
        public float startLifetime = 2;
        public float lifetime = 2;
        
        public float startSpeed = 5;
        public float speed = 5;
        
        public float startSize = 5;
        public float size = 5;
        
        public int rotation = 0;

        public ParticleData() {}
        public ParticleData(ParticleData p)
        {
            startColor = p.startColor;
            color = p.color;

            startLifetime = p.startLifetime;
            lifetime = p.lifetime;

            startSpeed = p.startSpeed;
            speed = p.speed;

            startSize = p.startSize;
            size = p.size;
        }
    }
    public class ParticleSystem : Component, IRenderable
    {
        public bool isPlaying = false;
        public int particlesPerSeconds = 4;
        
        public Color startColor = Color.White;
        public float startLifetime = 2;
        public float startSpeed = 5;
        public float startSize = 5;
        
        public List<Color> colorByLifetime = new();
        public List<float> speedByLifetime = new();
        public List<float> sizeByLifetime = new();

        public int minAngle = 0;
        public int maxAngle = 360;

        public int maxParticles = 1000;
        
        List<ParticleData> particles = new();
        
        public void Play() => isPlaying = true;
        public void Stop() => isPlaying = false;
        public void Clear() => particles.Clear();
        public void StopImmediate() { particles.Clear(); isPlaying = false; }
        public List<ParticleData> GetParticles() => particles;
        Random r = new();

        public void Emit(int count = 1)
        {
            for (int i = 0; i < count; i++)
            {
                if (particles.Count >= maxParticles)
                    continue;
                particles.Add(new ParticleData
                {
                    startColor = startColor,
                    color = startColor,

                    startLifetime = startLifetime,
                    lifetime = startLifetime,

                    startSpeed = startSpeed,
                    speed = startSpeed,

                    startSize = startSize,
                    size = startSize,

                    rotation = r.Next(minAngle, maxAngle)
                }); } }

        float time = 0;

        public override void Update()
        {
            if (particlesPerSeconds <= 0)
                return;

            if (maxAngle < 0 || maxAngle > 360)
                maxAngle = Math.Clamp(maxAngle, 0, 360);
            if (minAngle < 0 || minAngle > 360)
                minAngle = Math.Clamp(minAngle, 0, 360);

            if (isPlaying)
            {
                time += Time.deltaTime;
                float timeToSpawn = 1f / particlesPerSeconds;
                while (time >= timeToSpawn)
                {
                    time -= timeToSpawn;
                    Emit();
                }
            }

            if (particles.Count > 0)
            {
                for (int i = particles.Count - 1; i >= 0; i--)
                {
                    var p = particles[i];

                    p.lifetime -= Time.deltaTime;
                    if (p.lifetime <= 0)
                    {
                        particles.RemoveAt(i);
                        continue;
                    }
                    p.position += QMath.AngleToVector(p.rotation) * p.speed * 10f * Time.deltaTime;
                    float lifeT = 1f - p.lifetime / p.startLifetime;
                    lifeT = Math.Clamp(lifeT, 0f, 1f);

                    p.color = EvaluateByLifetime(p.startColor, colorByLifetime, lifeT);
                    p.speed = EvaluateByLifetime(p.startSpeed, speedByLifetime, lifeT);
                    p.size = EvaluateByLifetime(p.startSize, sizeByLifetime, lifeT);
                }
            }
        }
        static float EvaluateByLifetime(float start, List<float> values, float t)
        {
            if (values == null || values.Count == 0)
                return start;

            int total = values.Count + 1;

            float scaled = t * (total - 1);
            int index = (int)scaled;
            float localT = scaled - index;

            float from = index == 0
                ? start
                : values[index - 1];

            float to = index < values.Count
                ? values[index]
                : values[^1];

            return QMath.Lerp(from, to, localT);
        }
        static Color EvaluateByLifetime(Color start, List<Color> values, float t)
        {
            if (values == null || values.Count == 0)
                return start;

            int total = values.Count + 1;

            float scaled = t * (total - 1);
            int index = (int)scaled;
            float localT = scaled - index;

            Color from = index == 0
                ? start
                : values[index - 1];

            Color to = index < values.Count
                ? values[index]
                : values[^1];

            return QMath.Lerp(from, to, localT);
        }

        public void Draw() { foreach (var p in particles) { QRenderer.DrawShape(transform.position + p.position, p.size, p.color.to01()); } }
    }
    #endregion
    #endregion
}
