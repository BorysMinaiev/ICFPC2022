use std::{collections::HashMap, path::Path};

use algo_lib::{
    collections::array_2d::Array2D, geometry::point::PointT, io::input::Input,
    strings::utils::vec2str,
};

#[derive(Clone, Copy, Default, Debug)]
struct Color([u8; 4]);

impl Color {
    pub fn dist(&self, another: &Self) -> f64 {
        let mut res = 0.0;
        for i in 0..self.0.len() {
            let delta = self.0[i] as f64 - another.0[i] as f64;
            res += delta * delta;
        }
        res.sqrt()
    }
}

fn get_pixel_distance(my: &Array2D<Color>, expected: &Array2D<Color>) -> f64 {
    let mut res = 0.0;
    for i in 0..my.len() {
        for j in 0..my[i].len() {
            res += my[i][j].dist(&expected[i][j]);
        }
    }
    res * 0.005
}

type RectId = Vec<u8>;
type Point = PointT<i32>;

#[derive(Clone, Debug)]
enum Op {
    CutPoint(RectId, Point),
    Color(RectId, Color),
    CutY(RectId, i32),
}

fn remove_prefix(s: Vec<u8>, c: char) -> Vec<u8> {
    assert!(s[0] == c as u8);
    s[1..].to_vec()
}

fn remove_suffix(s: Vec<u8>, c: char) -> Vec<u8> {
    let n = s.len() - 1;
    assert!(s[n] == c as u8);
    s[..n].to_vec()
}

fn read_id(input: &mut Input) -> RectId {
    let id = input.string();
    assert!(id[0] == b'[');
    assert!(id[id.len() - 1] == b']');
    let id = remove_suffix(remove_prefix(id, '['), ']');
    id
}

fn parse_mid_int(s: &[u8]) -> i32 {
    let mut start = 0;
    while !(s[start] >= b'0' && s[start] <= b'9') {
        start += 1;
    }
    let mut end = s.len() - 1;
    while !(s[end] >= b'0' && s[end] <= b'9') {
        end -= 1;
    }
    vec2str(&s[start..=end]).parse().unwrap()
}

fn read_next_int(input: &mut Input) -> i32 {
    let s = input.string();
    parse_mid_int(&s)
}

fn read_color(input: &mut Input) -> Color {
    let r = read_next_int(input) as u8;
    let g = read_next_int(input) as u8;
    let b = read_next_int(input) as u8;
    let a = read_next_int(input) as u8;
    Color([r, g, b, a])
}

fn read_submit(path: &str) -> Vec<Op> {
    let mut res = vec![];
    let mut input = Input::new_file(path);
    while input.has_more_elements() {
        let cmd = input.string_as_string();
        if cmd == "cut" {
            let id = read_id(&mut input);
            let token = input.string();
            if token[1] == b'Y' || token[1] == b'X' {
                assert!(token[1] == b'Y');
                let y = read_next_int(&mut input);
                res.push(Op::CutY(id, y))
            } else {
                let x = parse_mid_int(&token);
                let y = read_next_int(&mut input);

                res.push(Op::CutPoint(id, Point::new(x, y)));
            }
        } else if cmd == "color" {
            let id = read_id(&mut input);
            let color = read_color(&mut input);
            res.push(Op::Color(id, color));
        } else {
            assert!(false);
        }
    }
    res
}

struct OpResult {
    picture: Array2D<Color>,
    cost: f64,
}

#[derive(Clone, Copy)]
struct Rect {
    from: Point,
    to: Point,
}

impl Rect {
    pub fn new(from: Point, to: Point) -> Self {
        Self { from, to }
    }

    pub fn size(&self) -> f64 {
        let dx = (self.to.x - self.from.x) as f64;
        let dy = (self.to.y - self.from.y) as f64;
        dx * dy
    }
}

fn get_start_id() -> RectId {
    vec![b'0']
}

fn apply_ops(ops: &[Op], n: usize, m: usize) -> OpResult {
    let canvas_size = (n as f64) * (m as f64);
    let mut a = Array2D::new(Color::default(), n, m);
    for i in 0..n {
        for j in 0..m {
            for k in 0..4 {
                a[i][j].0[k] = 255;
            }
        }
    }
    let mut rects = HashMap::new();
    let start_id = get_start_id();
    rects.insert(
        start_id,
        Rect::new(Point::ZERO, Point::new(n as i32, m as i32)),
    );
    let mut cost = 0.0;

    let gen_key = |id: &Vec<u8>, sub_id: usize| -> Vec<u8> {
        let mut res = id.clone();
        res.push(b'.');
        res.push(sub_id as u8 + b'0');
        res
    };

    for op in ops.iter() {
        match op {
            Op::CutPoint(id, p) => {
                let r = *rects.get(id).unwrap();
                let x1 = r.from.x;
                let x2 = p.x;
                let x3 = r.to.x;
                let y1 = r.from.y;
                let y2 = p.y;
                let y3 = r.to.y;

                rects.insert(
                    gen_key(id, 0),
                    Rect::new(Point::new(x1, y1), Point::new(x2, y2)),
                );
                rects.insert(
                    gen_key(id, 1),
                    Rect::new(Point::new(x2, y1), Point::new(x3, y2)),
                );
                rects.insert(
                    gen_key(id, 2),
                    Rect::new(Point::new(x2, y2), Point::new(x3, y3)),
                );
                rects.insert(
                    gen_key(id, 3),
                    Rect::new(Point::new(x1, y2), Point::new(x2, y3)),
                );
                rects.remove(id);
                cost += (10.0 * canvas_size / r.size()).round();
            }
            Op::Color(id, color) => {
                let r = *rects.get(id).unwrap();
                for x in r.from.x..r.to.x {
                    for y in r.from.y..r.to.y {
                        a[x as usize][y as usize] = *color;
                    }
                }
                cost += (5.0 * canvas_size / r.size()).round();
            }
            Op::CutY(id, split_y) => {
                let r = *rects.get(id).unwrap();
                rects.insert(
                    gen_key(id, 0),
                    Rect::new(r.from, Point::new(r.to.x, *split_y)),
                );
                rects.insert(
                    gen_key(id, 1),
                    Rect::new(Point::new(r.from.x, *split_y), r.to),
                );
                rects.remove(id);
                cost += (7.0 * canvas_size / r.size()).round();
            }
        }
    }
    dbg!(cost);
    OpResult { picture: a, cost }
}

fn save_image(a: &Array2D<Color>, path: &str) {
    let width = a[0].len();
    let height = a.len();

    let mut buffer = vec![];
    for x in 0..height {
        for y in 0..width {
            let c = a[y][height - 1 - x];
            for k in c.0.iter() {
                buffer.push(*k);
            }
        }
    }

    image::save_buffer(
        Path::new(path),
        &buffer,
        width as u32,
        height as u32,
        image::ColorType::Rgba8,
    )
    .unwrap()
}

fn solve_case(test_id: usize) {
    let mut input = Input::new_file(format!("../inputs/{}.txt", test_id));
    let m = input.usize();
    let n = input.usize();
    let mut expected = Array2D::new(Color::default(), n, m);
    for i in 0..m {
        for j in 0..n {
            for k in 0..4 {
                expected[j][m - i - 1].0[k] = input.u32() as u8;
            }
        }
    }
    let submit = read_submit(&format!("../outputs/{}.isl", test_id));
    let op_res = apply_ops(&submit, n, m);
    let dist = get_pixel_distance(&op_res.picture, &expected);
    dbg!(dist + op_res.cost);
    save_image(&op_res.picture, &format!("../images/{}.res.png", test_id))
}

fn main() {
    solve_case(7)
}