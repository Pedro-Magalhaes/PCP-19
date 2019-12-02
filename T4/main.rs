use std::rc::Rc;
use std::sync::{Mutex, Arc};
use std::thread;
use std::time::{Instant};

struct Tarefa {
    a: f64,
    b: f64,
    area: f64,
    calculated: bool,
}

// let THREADS = 4;

fn main() {

    let counter = Arc::new(Mutex::new(0.0));
    let mut handles = vec![];
    let inicio = 0.0;
    let fim = 1.0;
    let num_threads = 4;
    let incremento = f64::abs((fim - inicio)/num_threads as f64);
    let now = Instant::now();
    for i in 0..num_threads {
        let counter = Arc::clone(&counter);
        let a = incremento * i as f64 + inicio;
        let mut b = a + incremento;
        if i == num_threads-1 {
            b = fim;
        }
        let t = Tarefa {
            a: a ,
            b: b,
            area: 0.0,
            calculated: false
        };
        let handle = thread::spawn(move || {
            let mut num = counter.lock().unwrap();

            // *num += calcula_area1(t,&fx1);
            *num += calcula_area1b(t,&fx2b);
        });
        handles.push(handle);
    }
    for handle in handles {
        handle.join().unwrap();
    }
    let end = Instant::now();
    println!("Result: {}", *counter.lock().unwrap());
    println!("tempo {:?} secs",end.duration_since(now).as_secs_f64());
    // let area = calcula_area1(t,&x2);
    // println!("Area calculada {}",area);
}

fn x2(x:f64) -> f64 {
    return x * x;
}

// Funcoes com carga de processamento relevante
fn fx1(x:f64) -> f64 {
    let mut res: f64 = 0.0;
    for i in 0..10000 {
        res += f64::exp(3.0*x) * f64::sin(2.0*x);
    }
    return res;
}

fn fx2(x:f64) -> f64 {
    let mut res: f64 = 0.0;
    for i in 0..100000 {
        res += f64::exp(x);
    }
    return res;
}

fn fx2b(x:f64, a: f64, b:f64) -> f64 {

    let mut res: f64 = 0.0;
    let step = 0.0001;
    let num_steps = (f64::abs(b-a)/step) as u32;
    let mut last_y: f64 = f64::exp(a);
    for i in 1..num_steps {
        let mut inc = (i as f64) *step;
        let area = ((last_y + f64::exp(a + (i as f64) *step)) * step) / 2.0;
        res += area;
    }
        // println!("{}",num_steps );
    return res ;
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
        // println!("rec");
        let t1 = Tarefa{ a: t.a, b: meio, area: 0.0, calculated: false };
        let t2 = Tarefa { a: meio, b: t.b, area: 0.0, calculated: false };
        area_trapezios = calcula_area1(t1,f) + calcula_area1(t2,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }
    return area_trapezios;
}


fn calcula_area1b( t: Tarefa, f: &dyn Fn(f64,f64,f64) -> f64) -> f64{
    let h = f64::abs(t.b-t.a);
    let meio = (t.a+t.b)/2.0;
    let hmeio = h/2.0;
    
    // let fa = f64::abs(f(t.a));
    // let fb = f64::abs(f(t.b));
    // let fm = f64::abs(f(meio));
    let area_trapezio_maior =  fx2b(1.0,t.a,t.b);
    let mut area_trapezios =  fx2b(1.0,t.a,meio) + fx2b(1.0,meio,t.b);
    
    if f64::abs(area_trapezio_maior - area_trapezios) > 0.01 {
        // println!("rec");
        let t1 = Tarefa{ a: t.a, b: meio, area: 0.0, calculated: false };
        let t2 = Tarefa { a: meio, b: t.b, area: 0.0, calculated: false };
        area_trapezios = calcula_area1b(t1,f) + calcula_area1b(t2,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }
    return area_trapezios;
}
