use std::rc::Rc;
use std::sync::{Mutex, Arc};
use std::thread;
use std::time::{Instant};
use std::env;

struct Tarefa {
    a: f64,
    b: f64,
    area: f64,
    calculated: bool,
}

// let THREADS = 4;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 4 {
         println!("Erro, os parametros necessários não foram informados.\
                \nO Programa recebe os parametros nesta ordem:\
                \n\t<numero de threads>\n\t<valor de 'a'>\n\t<valor de 'b'>\
                \n\t<tolerancia>");
        println!("\nNumero de parametros recebidos: {}\n\n", args.len());
        return;
    }
    let counter = Arc::new(Mutex::new(0.0));
    let mut handles = vec![];
    let num_threads: u32 = args[1].parse().expect("Not a number");
    let inicio: f64 = args[2].parse().expect("Not a number");
    let fim: f64 = args[3].parse().expect("Not a number");
    let tol: f64 = args[4].parse().expect("Not a number");

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
        let handle = thread::spawn( move || {
            // *num += calcula_area1(t,&fx1);
            let area = calcula_area1(t,tol,&fx);

            let mut num = counter.lock().unwrap();

            *num += area;

            // println!("Terminei com num {}", *num);
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


fn fx(x: f64) -> f64 {
    return f64::exp(x) * f64::cos(x);
}

fn calcula_area1( t: Tarefa, tol: f64, f: &dyn Fn(f64) -> f64) -> f64{
    // println!("Thread {:?} trabalhando",thread::current().id());
    let h = f64::abs(t.b-t.a);
    let meio = (t.a+t.b)/2.0;
    let hmeio = h/2.0;
    
    let fa = f(t.a);
    let fb = f(t.b);
    let fm = f(meio);
    let area_trapezio_maior =  ((fa + fb) * h) / 2.0;
    let mut area_trapezios =  (((fa + fm) * hmeio) / 2.0 )+ (((fb + fm) * hmeio) / 2.0);
    
    if f64::abs(area_trapezio_maior - area_trapezios) > tol {
        // println!("rec");
        let t1 = Tarefa{ a: t.a, b: meio, area: 0.0, calculated: false };
        let t2 = Tarefa { a: meio, b: t.b, area: 0.0, calculated: false };
        area_trapezios = calcula_area1(t1,tol,f) + calcula_area1(t2,tol,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }
    return area_trapezios;
}
