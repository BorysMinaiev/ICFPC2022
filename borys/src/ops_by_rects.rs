use algo_lib::collections::array_2d::Array2D;

use crate::{
    color_corner::color_corner, merger::MergeResult, op::Op, solver::SolutionRect,
    test_case::TestCase, utils::p, Point,
};

pub fn gen_ops_by_solution_rects(
    rects: &[SolutionRect],
    field_n: usize,
    field_m: usize,
    merge_result: &MergeResult,
    test_case: &TestCase,
) -> Vec<Op> {
    let mut res = merge_result.ops.clone();
    let n = rects.len();
    let mut ok_after = Array2D::new(true, n, n);
    for i in 0..n {
        for j in 0..n {
            if rects[i].from.x >= rects[j].to.x || rects[i].from.y >= rects[j].to.y {
                ok_after[i][j] = true;
            }
        }
    }
    let mut need_more_before = vec![0; n];
    for i in 0..n {
        for j in 0..n {
            if !ok_after[i][j] {
                need_more_before[j] += 1;
            }
        }
    }
    let mut queue = vec![];
    let mut used = vec![false; n];
    loop {
        let mut changed = false;
        for v in 0..n {
            if need_more_before[v] == 0 && !used[v] {
                changed = true;
                used[v] = true;
                queue.push(v);
                for j in 0..n {
                    if !ok_after[v][j] {
                        need_more_before[j] -= 1;
                    }
                }
            }
        }
        if !changed {
            break;
        }
    }
    assert!(queue.len() == n);
    let mut cur_whole_id = merge_result.last_block_id;
    for &rect_id in queue.iter() {
        let r = rects[rect_id];
        let to_color = color_corner(field_n, field_m, r.from, cur_whole_id, r.color, test_case);
        res.extend(to_color.ops);
        cur_whole_id = to_color.cur_whole_id;
    }

    res
}

pub fn gen_rects_by_ops(ops: &[Op], n: usize, m: usize) -> Vec<SolutionRect> {
    let mut res = vec![];
    let mut last_point = Point::ZERO;
    for op in ops.iter() {
        match op {
            Op::CutPoint(_, p) => last_point = *p,
            Op::Color(_, color) => res.push(SolutionRect {
                from: last_point,
                to: p(n, m),
                color: *color,
            }),
            Op::CutY(_, y) => last_point = Point::new(0, *y),
            Op::CutX(_, x) => last_point = Point::new(*x, 0),
            Op::Merge(_, _) => {}
            Op::Swap(_, _) => {}
        }
    }
    res
}
