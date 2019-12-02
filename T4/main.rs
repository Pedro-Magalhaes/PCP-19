struct Tarefa {
    a: f64,
    b: f64,
    area: f64,
    calculated: bool,
}


fn main() {
    let t = Tarefa{
        a:1.0, b: 2.0, area: 0.0, calculated: false
    };
    let area = calcula_area1(t,&x2);
    println!("Area calculada {}",area);
}

fn x2(x:f64) -> f64 {
    return x * x;
}

fn calcula_area1( t: Tarefa, f: &dyn Fn(f64) -> f64) -> f64{
    let h = f64::abs(t.b-t.a);
    let meio = (t.a+t.b)/2.0;
    let hmeio = h/2.0;
    
    let fa = f64::abs(f(t.a));
    let fb = f64::abs(f(t.b));
    let fm = f64::abs(f(meio));
    let area_trapezio_maior =  ((fa + fb) * h) / 2.0;
    let mut area_trapezios =  (((fa + fm) * hmeio) / 2.0 )+ (((fb + fm) * hmeio) / 2.0);
    
    if f64::abs(area_trapezio_maior - area_trapezios) > 0.001 {
        println!("rec");
        let t1 = Tarefa{ a: t.a, b: meio, area: 0.0, calculated: false };
        let t2 = Tarefa { a: meio, b: t.b, area: 0.0, calculated: false };
        area_trapezios = calcula_area1(t1,f) + calcula_area1(t2,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }
    return area_trapezios;
}
