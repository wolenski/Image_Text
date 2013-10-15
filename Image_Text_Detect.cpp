
    valid_flag = true;
    return ACTION_SKIP_CHILD;
}
EActionResult LayoutCtrlDefault::process_tag_img(DomNode* node, bool proc_start)
{
    valid_flag = true;
    if(!proc_start){
    	int detect_flag = true;

    	detect_img_text_struct(node);
    }
    return ACTION_OK;
}


int count_non_puretext_child(DomNode *root_node){
	if(!root_node || root_node->is_null()){
		return 0;
	}
	int count = 0;
	int loop = 0;
	DomNode child = root_node->get_first_child();
	while(!child.is_null()){
		if(TAG_PURETEXT!=child.get_type()){
			count++;
		}
		child = child.get_next_node();
		loop++;
		if(loop>50){
//			printf("NOTICE! Maybe infinite iterations!\n");
			break;
		}
	}
	return count;
}
/**
 * @brief sibling nodes must surrounds the image,
 *        upside,right,downside
 */
bool is_valid_sibling(DomNode *node, DomNode *sibling){
	bool valid_flag = false;
	/// information of the image node
	int height = node->get_height();
	int width = node->get_width();
	int img_pos_x = node->get_pos_x();
	int img_pos_y = node->get_pos_y();
    /// information of current node
	int hx = sibling->get_height();
	int wx = sibling->get_width();
	int pos_x = sibling->get_pos_x();
	int pos_y = sibling->get_pos_y();
	int text_size = sibling->get_sub_text_size();
    /// check size
	if(hx<5 || wx<5){
//		printf("size fail! \n");
		valid_flag = false;
		return valid_flag;
	}
	/// check position
	if (pos_x>=img_pos_x+width && pos_y+hx>img_pos_y && pos_y<=img_pos_y+height && text_size>=4){
		valid_flag = true;
	}else if(pos_y+hx<=img_pos_y && text_size>=4){
		valid_flag = true;
	}else if(pos_y>=img_pos_y && sibling->get_descendant_count(TAG_A)>0){
		valid_flag = true;
	}

	return valid_flag;
}

int LayoutCtrlDefault::detect_img_text_struct(DomNode *node){
	if(node->is_null() || TAG_IMG != node->get_type()){
		return -1;
	}
	/// parameters of the image
	int height = node->get_height();
	int width = node->get_width();
	int img_pos_x = node->get_pos_x();
	int img_pos_y = node->get_pos_y();
	/// limit the size of the image
	if(height<=30 || width<=30 || width>=480){
		printf("image size fail!\n");
		return -1;
	}
	double hw_ratio = height/(double)width;
	double wh_ratio = width/(double)height;
	if(hw_ratio>1.5 || wh_ratio>1.5){
//		return -1;
	}
	/// get float attribute
    bsl::string css_float_str;
    bool b_left_flag = false;
	node->get_style(CSS_PROP_FLOAT, css_float_str);
	if(css_float_str == "left"){
		b_left_flag = true;
	}
	/// find the root node of normal block
	int tmp_int = 0;
	int depth = 0;
	DomNode root_node = *node;
	while(tmp_int==0){
		if(depth > 10){
			return -1;
		}
		if(root_node.is_null()){
			return -1;
		}
	    root_node.get_attr("tc-normal-root",&tmp_int,TYPE_INT);
        if(tmp_int != 0){
        	break;
        }
	    root_node = root_node.get_parent();
	    depth++;
	}
    if(root_node.is_null()){
    	return -1;
    }
    /// Judge if TAG_IMG is wrapped by a single parent node
	DomNode img_node = *node;
	DomNode img_pare_node = img_node.get_parent();
	bool is_wrapped_by_tag_a = false;
	while(count_non_puretext_child(&img_pare_node) == 1){
		img_node = img_pare_node;
		img_pare_node = img_node.get_parent();
	}
    /// image is a single node
	if(img_node.get_id()<=root_node.get_id()){
//		printf("no upper node to add tc-clear!\n");
		return -1;
	}
	/// information of the parent node
	int pare_hx = img_pare_node.get_height();
	int pare_wx = img_pare_node.get_width();
	int pare_pos_x = img_pare_node.get_pos_x();
	int pare_pos_y = img_pare_node.get_pos_y();
	/// make sure the image is the most left node
    if(!b_left_flag){
    	if(img_pos_x<pare_pos_x){
    		return -1;
    	}
    }
    
	/**
	 * @brief log feature
	 *
	 */
	// get block xpath
	bsl::string xpath_str;
	img_pare_node.get_xpath_from_node(xpath_str);
	// get src url
	bsl::string src_url;
	root_node.get_attr("tc-src",src_url);
	// decode url
	char url_buf[2048] = {0};
	url_decode(src_url.c_str(),url_buf,sizeof(url_buf));
	// get feature value
	int anchor_count = img_pare_node.get_descendant_count(TAG_A);
	int sub_text_size = img_pare_node.get_sub_text_size();
	html_tag_type_t tag_t = node->get_parent().get_type();
	int in_tag_a = (TAG_A == tag_t)?1:0;
	int float_left = b_left_flag?1:0;
	// should have more than 2 words
	if(sub_text_size < 2){
        return -1;
    }

    int block_pos_x = pare_pos_x;
	int block_pos_y = pare_pos_y;
	int block_hx = pare_hx;
	int block_wx = pare_wx;
	int img_hx = height;
	int img_wx = width;
	/// avoid overflow or wrong result, e.g. INF,NaN
    if(block_wx < img_wx){
        block_wx = img_wx;
	}
    if(block_hx < img_hx){
        block_hx = img_hx;
	}
	
	/**
	 * @biref machine learning method
	 *        tree models by NLP
	 */
	int margin_left = img_pos_x - block_pos_x;
	int margin_top = img_pos_y - block_pos_y;
	int margin_right = block_wx - img_wx - margin_left;
	int margin_bottom = block_hx - img_hx - margin_top;
	double block_area = block_wx * block_hx;
	double img_area = img_wx * img_hx;
	// margin area
	double margin_left_area = margin_left * img_hx;
	double margin_top_area = margin_top * block_wx;
	double margin_right_area = margin_right * img_hx;
	double margin_bottom_area = margin_bottom * block_wx;

	// ##############################
	// ### normalization
	// ##############################
	double fea_block_wx = float(block_wx) / 1024;
	double fea_block_hx = float(block_hx) / 1024;
	double fea_img_wx = float(img_wx) / 1024;
	double fea_img_hx = float(img_hx) / 1024;
	double fea_anchor_count = float(anchor_count) / (sub_text_size + 1);
	// # regard one word as 14px * 14px
	double fea_sub_text_size = float(sub_text_size) * 200 / (block_area - img_area - margin_left_area + 1);
	double fea_in_tag_a = float(in_tag_a);
	double fea_float_left = float(float_left);
	// ##############################
	// ## generalize new feature
	// ##############################
	double fea_img_block_ratio_wx = fea_img_wx / fea_block_wx;
	double fea_img_block_ratio_hx = fea_img_hx / fea_block_hx;
	double fea_img_block_ratio_margin_left = float(margin_left) / block_wx;
	double fea_img_block_ratio_margin_top = float(margin_top) / block_hx;
	double fea_img_block_ratio_margin_right = float(margin_right) / block_wx;
	double fea_img_block_ratio_margin_bottom = float(margin_bottom) / block_hx;
	double fea_img_block_ratio_area = fea_img_block_ratio_wx * fea_img_block_ratio_hx;
	double fea_img_block_ratio_margin_left_area = float(margin_left_area) / block_area;
	double fea_img_block_ratio_margin_top_area = float(margin_top_area) / block_area;
	double fea_img_block_ratio_margin_right_area = float(margin_right_area) / block_area;
	double fea_img_block_ratio_margin_bottom_area = float(margin_bottom_area) / block_area;
	
	std::vector<double> fea_vec;
	fea_vec.push_back(fea_block_wx);
	fea_vec.push_back(fea_block_hx);
	fea_vec.push_back(fea_img_wx);
	fea_vec.push_back(fea_img_hx);
	fea_vec.push_back(fea_anchor_count);
	fea_vec.push_back(fea_sub_text_size);
	fea_vec.push_back(fea_in_tag_a);
	fea_vec.push_back(fea_float_left);
	fea_vec.push_back(fea_img_block_ratio_wx);
	fea_vec.push_back(fea_img_block_ratio_hx);
	fea_vec.push_back(fea_img_block_ratio_margin_left);
	fea_vec.push_back(fea_img_block_ratio_margin_top);
	fea_vec.push_back(fea_img_block_ratio_margin_right);
	fea_vec.push_back(fea_img_block_ratio_margin_bottom);
	fea_vec.push_back(fea_img_block_ratio_area);
	fea_vec.push_back(fea_img_block_ratio_margin_left_area);
	fea_vec.push_back(fea_img_block_ratio_margin_top_area);
	fea_vec.push_back(fea_img_block_ratio_margin_right_area);
	fea_vec.push_back(fea_img_block_ratio_margin_bottom_area);
	
	double line[128] = {0};
	for(int i=0; i<fea_vec.size(); i++){
	    line[i] = fea_vec[i];
	}
    string model_file = "./conf/model.d";
	TreeLearner TLearner;
	if(0 != TLearner.LoadModelFull(model_file)){
	    cout << "fail to load model!\n";
	    return -1;
	}
	double ml_result = -1.0;
	if(0 != TLearner.OnlineTesting(line,ml_result,fea_vec.size())){
	    cout << "ERROR: Failed online testing!\n";
		return -1;
	}

	/**
	 * @biref manual method
	 */

	/// limit the height of the parent node
    DomNode child_node = img_pare_node.get_first_child();
    int out_hx = 0;
    int loop_num = 0;
    while(!child_node.is_null()){
    	if(TAG_PURETEXT != child_node.get_type() && child_node.get_id()!=img_node.get_id()){
			int c_hx = child_node.get_height();
			int c_wx = child_node.get_width();
			int c_pos_x = child_node.get_pos_x();
			int c_pos_y = child_node.get_pos_y();
			int c_text_size = child_node.get_sub_text_size();
			if(c_pos_y+c_hx<=img_pos_y && c_text_size>=4){
				out_hx += c_hx;
			}else if(c_pos_y>=img_pos_y){
				out_hx += c_hx;
			}
    	}
    	child_node = child_node.get_next_node();
    	loop_num++;
    	if(loop_num>50){
            printf("NOTICE! Maybe infinite iteration!\n");
    		break;
    	}
    }
	double manual_result = 0;
	if(out_hx>2*height){
	    printf("image height too small! \n");
		return -1;
	}
	/// process sibling nodes
	DomNode sibling = img_node.get_next_node();
	int count_valid = 0;
	int total_text = 0;
	while(!sibling.is_null()){
		/// ignore pure text node
        if(TAG_PURETEXT!=sibling.get_type() && is_valid_sibling(node, &sibling)){
        	int text_size = sibling.get_sub_text_size();
        	count_valid++;
        	total_text += text_size;
         }
		sibling = sibling.get_next_node();
	}
	
	if(count_valid>0 && total_text>20){
	    manual_result = 1;
	}
	
	/**
	 * @biref output log
	 */
	FILE *fp = fopen("feature_test.txt","a");
	if(fp!=NULL){
		fprintf(fp,"url:%s\n",url_buf);
		fprintf(fp,"xpath:%s\n",xpath_str.c_str());
		fprintf(fp,"block_pos_x:%d,block_pos_y:%d,block_hx:%d,block_wx:%d,",pare_pos_x,pare_pos_y,pare_hx,pare_wx);
		fprintf(fp,"img_pos_x:%d,img_pos_y:%d,img_hx:%d,img_wx:%d,",img_pos_x,img_pos_y,height,width);
		fprintf(fp,"anchor_count:%d,sub_text_size:%d,in_tag_a:%d,float_left:%d\n",anchor_count,sub_text_size,in_tag_a,float_left);
		fclose(fp);
		fp = NULL;
	}
    
	fp = fopen("classify_ml.txt","a");
	if(fp!=NULL){
		fprintf(fp,"tree-models:%d\n",int(ml_result));
		fprintf(fp,"url:%s\n",url_buf);
		fprintf(fp,"xpath:%s\n",xpath_str.c_str());
		fprintf(fp,"block_pos_x:%d,block_pos_y:%d,block_hx:%d,block_wx:%d,",pare_pos_x,pare_pos_y,pare_hx,pare_wx);
		fprintf(fp,"img_pos_x:%d,img_pos_y:%d,img_hx:%d,img_wx:%d,",img_pos_x,img_pos_y,height,width);
		fprintf(fp,"anchor_count:%d,sub_text_size:%d,in_tag_a:%d,float_left:%d\n",anchor_count,sub_text_size,in_tag_a,float_left);
		fclose(fp);
		fp = NULL;
	}
	
	fp = fopen("classify_manual.txt","a");
	if(fp!=NULL){
		fprintf(fp,"manual:%d\n",int(manual_result));
		fprintf(fp,"url:%s\n",url_buf);
		fprintf(fp,"xpath:%s\n",xpath_str.c_str());
		fprintf(fp,"block_pos_x:%d,block_pos_y:%d,block_hx:%d,block_wx:%d,",pare_pos_x,pare_pos_y,pare_hx,pare_wx);
		fprintf(fp,"img_pos_x:%d,img_pos_y:%d,img_hx:%d,img_wx:%d,",img_pos_x,img_pos_y,height,width);
		fprintf(fp,"anchor_count:%d,sub_text_size:%d,in_tag_a:%d,float_left:%d\n",anchor_count,sub_text_size,in_tag_a,float_left);
		fclose(fp);
		fp = NULL;
	}
	
	fp = fopen("classify_feature.txt","a");
	if(fp!=NULL){
		fprintf(fp,"%d\t",int(ml_result));
	    for(int i=0; i<fea_vec.size(); i++){
	        fprintf(fp,"%d:%f\t",i,fea_vec[i]);
	    }
		fprintf(fp,"\n");		
		fclose(fp);
		fp = NULL;
	}
	return 0;
}

