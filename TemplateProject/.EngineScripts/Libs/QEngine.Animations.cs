using System;
using System.Collections.Generic;

using QEngine.GUI;

namespace QEngine.Animations
{
    public class Animation
    {
        List<Sprite> Frames = new();
        int framesPerSecond = 1;
        public void AddFrame(Sprite frame) => Frames.Add(frame);
        public void AddFrames(List<Sprite> frames) { foreach (var f in frames) Frames.Add(f); }
        public Sprite GetFrame(int index) => Frames[Math.Clamp(index, 0, Frames.Count - 1)];
        public int GetFPS() => framesPerSecond;
        public void SetFPS(int fps) => framesPerSecond = fps;
        public List<Sprite> GetFrames() => Frames;
        public int GetFramesCount() => Frames.Count;
        public void Clear() => Frames.Clear();
        public void RemoveFrame(int index) => Frames.RemoveAt(Math.Clamp(index, 0, Frames.Count - 1));
    }
    public class Animator : Component
    {
        public bool isPaused = false;
        Animation current = null;
        int currentFrame = 0;
        
        float time = 0;
        
        Dictionary<string, Animation> animations = new();
        
        public Animation GetAnimation(string name) => animations[name];
        public Animation AddAnimation(string name) { animations.Add(name, new()); return animations[name]; }
        public void RemoveAnimation(string name) => animations.Remove(name);
        public void AddFrame(string name, Sprite frame) => animations[name].AddFrame(frame);
        public void AddFrames(string name, List<Sprite> frames) => animations[name].AddFrames(frames);
        
        public void Play(string name) { if (current != animations[name]) current = animations[name]; isPaused = false; }
        public void Pause() => isPaused = true;
        public void UnPause() => isPaused = false;
        public void Reset() => currentFrame = 0;
        
        public override void Update()
        {
            if (current == null || isPaused) return;
            time += 1f / 60f;
            float timeToNext = 1f / current.GetFPS();
            while (time >= timeToNext)
            {
                currentFrame++; time -= timeToNext;
                if (currentFrame >= current.GetFrames().Count) { currentFrame = 0; }
                if (gameObject.TryGetComponent(out Image img))
                { img.sprite = current.GetFrame(currentFrame); }
            }
        }
    }
}
