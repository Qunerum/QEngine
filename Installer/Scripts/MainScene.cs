using QEngine;
using QEngine.GUI;
using QEngine.Input;
using QEngine.Text;

public class MainScene : QEScene
{
    public override void Init()
    {
        Game.title = "Your Game Title";
        Game.background = Color.Red;
        var t = new GameObject().
            AddComponent<Text>().gameObject.
            AddComponent<ParticleSystem>().gameObject.
            AddComponent<tes> ();
        //TestFont();
    }
    void TestFont()
    {
        var t = new GameObject("Object").AddComponent<Text>();
        t.transform.position = new(-620, 100);
        t.text = "Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm\n" +
                 "Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz\n" +
                 "`~ 1! 2@ 3# 4$ 5% 6^ 7& 8* 9( 0) -_ =+\n" +
                 "[{ ]} \\| ;: '\" ,< .> /?";
    }
}

public class tes : QEScript
{
    Text t;
    ParticleSystem ps;
    bool up, down;
    bool left, right;

    public override void Init()
    {
        t = gameObject.GetComponent<Text>();
        ps = gameObject.GetComponent<ParticleSystem>();
        ps.particlesPerSeconds = 0;
        ps.Play();
    }
    public override void Update()
    {
        t.text = $"\n\n\nPPS: {ps.particlesPerSeconds}\nSL: {ps.startLifetime}";
        t.color = Color.White;
        if (Input.Read(Key.W)) { if (!up) { up = true; ps.particlesPerSeconds += 100;} } else { up = false; }
        if (Input.Read(Key.S)) { if (!down) { down = true; ps.particlesPerSeconds -= 100;} } else { down = false; }
        if (Input.Read(Key.A)) { if (!left) { left = true; ps.startLifetime--;} } else { left = false; }
        if (Input.Read(Key.D)) { if (!right) { right = true; ps.startLifetime++;} } else { right = false; } 
    }
}