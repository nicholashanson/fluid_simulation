def insert_profiling_abs( object_path, target_path, out, gpu_out, state_out, GPU, state, sections ):

    profile_steps = 10
    
    with open( object_path, 'r', encoding = 'utf-8' ) as file:
        original_content = file.readlines()

    profiled_content = []

    profiled_content.append( '#include <fstream>\n\n' )

    for original_line in original_content:

        stripped_line = original_line.lstrip()

        indent = original_line[:len( original_line) - len( stripped_line ) ]

        matched = False

        stripped_line = stripped_line.rstrip()

        for section in sections:

            if stripped_line == 'while ( !glfwWindowShouldClose( window ) ) {':
                profiled_content.append(f'{ indent }for ( size_t i = 0; i < { profile_steps }; ++i ) {{\n' )
                matched = True
                break

            elif stripped_line == 'bool simulation_running = false;':
                profiled_content.append( f'{ indent }bool simulation_running = true;\n' )
                matched = True
                break

            elif stripped_line == f"// start loop":
                if state: 
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ state_out }\");\n" )
                elif GPU:
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ gpu_out }\");\n" )
                else:
                    profiled_content.append( f"{ indent }std::ofstream file(\"{ out }\");\n" )

                profiled_content.append( f"{ indent }auto start_loop = std::chrono::high_resolution_clock::now();\n" )
                matched = True
                break
                
            elif stripped_line == f"// start { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto start_{ cpp_var } = std::chrono::high_resolution_clock::now();\n" )
                profiled_content.append( f"{ indent }file << \"{ section } start: \" << std::chrono::duration_cast<std::chrono::microseconds>( start_{ cpp_var } - start_loop ).count() << std::endl;;\n"  )
                matched = True
                break

            elif stripped_line == f"// end { section }":
                cpp_var = section.replace( ' ', '_' )
                profiled_content.append( f"{ indent }auto end_{ cpp_var } = std::chrono::high_resolution_clock::now();\n" )
                profiled_content.append( f"{ indent }file << \"{ section }: \" <<  std::chrono::duration_cast<std::chrono::microseconds>( end_{ cpp_var } - start_{ cpp_var } ).count() << std::endl; \n" ) 
                matched = True
                break
        
        if matched == False:
            profiled_content.append( original_line )
    
    with open( target_path, 'w', encoding = 'utf-8' ) as file:
        file.writelines( profiled_content )