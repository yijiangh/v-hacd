from __future__ import print_function

import os
import argparse
from datetime import datetime
from py_vhacd import compute_convex_decomp

def main(precompute=False):
    parser = argparse.ArgumentParser()
    # rack.obj | cage.obj |
    parser.add_argument('-i', '--input', default='rack.obj', help='The name of the problem to solve')
    parser.add_argument('-of', '--output_format', default='wrl', help='the output file format (obj, wrl, off)')
    parser.add_argument('-res', '--resolution', default=100000, help='maximum number of voxels generated during the voxelization stage (default 100000, range 10,000-64,000,000)')
    parser.add_argument('-v', '--verbose', action='store_true', help='verbose (default false).')
    parser.add_argument('-wo', '--write_output', action='store_true', help='If generate obj/wrl result file. (default false)')
    parser.add_argument('-wl', '--write_log', action='store_true', help='If generate log file. (default false)')

    args = parser.parse_args()
    print('Arguments:', args)

    print('test started!')

    now = datetime.now()
    date_time = now.strftime("%m-%d-%Y_%H-%M-%S")

    root_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(root_dir, '..', 'output')
    log_dir = os.path.join(root_dir, '..', 'log')

    input_path = os.path.join(root_dir, '..', 'input', args.input)
    output_path = ''
    log_path = ''

    if args.write_output:
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        output_path = os.path.join(output_dir, args.input + '_' + date_time + '.' + args.output_format)
    if args.write_log:
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        log_path = os.path.join(log_dir, args.input + '_' + date_time + '.log')

    sucess, mesh_verts, mesh_faces = compute_convex_decomp(input_path, output_path, log_path, resolution=int(args.resolution), verbose=args.verbose)

    print('\n\n******************************')
    print('sucess: {}'.format(sucess == 0))
    print('# of convex hulls generated: {}'.format(len(mesh_verts)))
    print('mesh_verts: {}'.format(mesh_verts))
    print('mesh_faces: {}'.format(mesh_faces))

if __name__ == '__main__':
    main()
